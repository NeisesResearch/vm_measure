/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/* Linux kernel module for cross vm connection.
 * This allows the parsing of cross vm connections advertised as a PCI device.
 * Cross vm connections being a combination of events and shared memory regions
 * between linux processes and camkes components). This module utilises userspace io
 * to allow linux processes to mmap the pci bars.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/uio_driver.h>
#include <asm/io.h>

/* This is an 'abitrary' custom device id we use to match on our dataports
 * pci driver */
#define PCI_CONNECTOR_DEVICE_ID 0xa111
#define MAX_CONNECTOR_DEVICES 32

#define DEVICE_NAME_REGISTER_OFFSET 2
#define DEVICE_NAME_MAX_LEN 50
typedef struct connector_dev_node {
    struct pci_dev *dev;
    struct uio_info *uio;
    char dev_name[DEVICE_NAME_MAX_LEN];
} connector_dev_node_t;

connector_dev_node_t *devices[MAX_CONNECTOR_DEVICES];
unsigned int current_free_dev = 0;

enum MeasurementState
{
    Waiting,
    CollectingModuleMeasurements,
    SendingModuleMeasurements,
    CollectingLinuxMeasurements,
    SendingLinuxMeasurements
};

typedef struct MeasurementManager
{
    enum MeasurementState state;
    uint32_t** measurements;
    uint32_t memory;
    uint32_t numMeasurements;
} MeasurementManager;
MeasurementManager measurementManager = { Waiting, NULL, 0 };

u32* signoff = (u32*)"DEADBEEF";

static void dataportRead(u32* result)
{
    int i;
    phys_addr_t internal_addr = devices[0]->uio->mem[1].internal_addr;
    for (i=0; i<1024; i++)
    {
        result[i] = readl(internal_addr);
        internal_addr = (u32*)internal_addr + 1;
    }
}

static void dataportPrintData(void)
{
    int i;
    int j;
    u32* input = kmalloc(4096, GFP_KERNEL);
    dataportRead(input);

    // print 8 rows of 128
    printk("Dataport Contents:\n");
    u8* inputBytes = (u8*)input;
    for(i=0;i<32;i++)
    {
        u8* thisRow = kmalloc(129, GFP_KERNEL);
        for(j=0;j<128;j++)
        {
           thisRow[j] = inputBytes[128*i + j]; 
        }
        thisRow[128] = '\0';
        printk("%s\n", (char*)thisRow);
    }
    kfree(input);
}

static void send_ready_signal(void)
{
    uint32_t* event_bar = devices[0]->uio->mem[0].internal_addr;
    writel(1, &event_bar[0]);
}

static void dataportWrite(u32* input, int length)
{
    if(length > 1024)
    {
        printk("dataportWrite: length too large: input truncated");
    }
    int i;
    phys_addr_t internal_addr = devices[0]->uio->mem[1].internal_addr;
    for (i=0; i<length && i<1024; i++)
    {
        writel(input[i], internal_addr);
        internal_addr = (u32*)internal_addr + 1;
    }
}


// size in bytes
static void sendNextModuleMeasurement(void)
{
    u32* thisMeasurement = measurementManager.measurements[measurementManager.memory];
    if(thisMeasurement == NULL)
    {
        measurementManager.state = Waiting;
        dataportWrite(signoff, 2);
    }
    else
    {
        printk("Sending Module Measurement %d...\n", measurementManager.memory);
        dataportWrite(measurementManager.measurements[measurementManager.memory], 64);
        measurementManager.memory = measurementManager.memory + 1;
    }
    send_ready_signal();
}

static void measureModules(void)
{
    printk("Got a measurement request...\n");
    //dataportPrintData();

    /*
    ** Assume there can be at most 100 modules
    ** Assume no module has a longer name than 256 characters
    ** :shrug:
    */
    measurementManager.measurements = kzalloc(100 * sizeof(uint32_t*), GFP_KERNEL);
    measurementManager.numMeasurements = 0;
    int moduleNameSize = 256;


    struct module *mod;
    struct module_layout myLayout;
    int it = 0;
//   mutex_lock(&module_mutex);
    list_for_each_entry(mod, &THIS_MODULE->list, list)
            printk(KERN_INFO, "%s\n", mod->name);
            char* thisName = mod->name;
            // for now, send only the name of the modules, padded with zeroes to 256 bytes
            char* sendName = kmalloc(moduleNameSize, GFP_KERNEL);
            for(it=0; it<strlen(thisName); it++)
            {
                sendName[it] = thisName[it];
            }
            for(it=strlen(thisName); it<moduleNameSize; it++)
            {
                sendName[it] = "0";
            }
            measurementManager.measurements[measurementManager.numMeasurements] = (u32*)sendName;
            measurementManager.numMeasurements++;
//    mutex_unlock(&module_mutex);

    measurementManager.memory = 0;
    measurementManager.state = SendingModuleMeasurements;
    sendNextModuleMeasurement();
}

static irqreturn_t connector_event_handler(int irq, struct uio_info *dev_info)
{
    uint32_t *event_bar = dev_info->mem[0].internal_addr;
    u32 val;
    val = readl(&event_bar[1]);
    if (val == 0) {
        /* No event - IRQ wasn't for us */
        return IRQ_NONE;
    }
    /* Clear the register and return IRQ
     * TODO: Currently avoiding IRQ count value - we might want to use it? */
    writel(0, &event_bar[1]);


    //printk("\nGot an event!\n");
    switch(measurementManager.state)
    {
        case Waiting:
            printk("Measuring modules...\n");
            measurementManager.state = CollectingModuleMeasurements;
            measureModules();
            break;
        case CollectingModuleMeasurements:
            printk("Still measuring...\n");
            break;
        case SendingModuleMeasurements:
            sendNextModuleMeasurement();
            break;
        case CollectingLinuxMeasurements:
            break;
        case SendingLinuxMeasurements:
            break;
    }

    return IRQ_HANDLED;
}


static int connector_pci_probe(struct pci_dev *dev,
                               const struct pci_device_id *id)
{
    connector_dev_node_t *connector;
    struct uio_info *uio;
    uint32_t *event_bar;
    int num_bars;
    int err = 0;
    int unmap = 0;
    int i = 0;

    if (current_free_dev >= MAX_CONNECTOR_DEVICES) {
        printk("Failed to initialize connector: Managing maximum number of devices\n");
        return -ENODEV;
    }

    connector = kzalloc(sizeof(connector_dev_node_t), GFP_KERNEL);
    if (!connector) {
        printk("Failed to initalize connector\n");
        return -ENOMEM;
    }

    uio = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
    if (!uio) {
        printk("Failed to initialize uio information\n");
        kfree(connector);
        return -ENOMEM;
    }

    if (pci_enable_device(dev)) {
        goto free_alloc;
    }

    if (pci_request_regions(dev, "connector")) {
        goto disable_pci;
    }

    /* Set up the event bar - We assume the event bar is always BAR0
     * even if the PCI device does not use events */
    uio->mem[0].addr = pci_resource_start(dev, 0);
    if (!uio->mem[0].addr) {
        goto disable_pci;
    }
    uio->mem[0].size = pci_resource_len(dev, 0);
    uio->mem[0].internal_addr = pci_ioremap_bar(dev, 0);
    if (!uio->mem[0].internal_addr) {
        goto disable_pci;
    }
    uio->mem[0].memtype = UIO_MEM_PHYS;
    printk("Event Bar (dev-%d) initalised\n", current_free_dev);

    for (i = 1; i < MAX_UIO_MAPS; i++) {
        uio->mem[i].addr = pci_resource_start(dev, i);
        if (!uio->mem[i].addr) {
            /* We assume the first NULL bar is the end
             * Implying that all dataports are passed sequentially (i.e. no gaps) */
            break;
        }

        uio->mem[i].internal_addr = ioremap_cache(pci_resource_start(dev, i),
                                                  pci_resource_len(dev, i));
        if (!uio->mem[i].internal_addr) {
            err = 1;
            break;
        }
        uio->mem[i].size = pci_resource_len(dev, i);
        uio->mem[i].memtype = UIO_MEM_PHYS;
    }
    num_bars = i;
    if (err) {
        goto unmap_bars;
    }

    /* Register our IRQ handler */
    uio->irq = dev->irq;
    uio->irq_flags = IRQF_SHARED;
    uio->handler = connector_event_handler;

    connector->uio = uio;
    connector->dev = dev;
    event_bar = uio->mem[0].internal_addr;
    strncpy(connector->dev_name, (char *)&event_bar[DEVICE_NAME_REGISTER_OFFSET], DEVICE_NAME_MAX_LEN);
    uio->name = connector->dev_name;
    uio->version = "0.0.1";

    if (uio_register_device(&dev->dev, uio)) {
        goto unmap_bars;
    }

    printk("%d Dataports (dev-%d) initalised\n", num_bars, current_free_dev);
    pci_set_drvdata(dev, uio);
    devices[current_free_dev++] = connector;

    return 0;
unmap_bars:
    iounmap(uio->mem[0].internal_addr);
    for (unmap = 1; unmap < num_bars; unmap++) {
        iounmap(uio->mem[unmap].internal_addr);
    }
    pci_release_regions(dev);
disable_pci:
    pci_disable_device(dev);
free_alloc:
    kfree(uio);
    kfree(connector);
    return -ENODEV;
}

static void connector_pci_remove(struct pci_dev *dev)
{
    struct uio_info *uio = pci_get_drvdata(dev);
    uio_unregister_device(uio);
    pci_release_regions(dev);
    pci_disable_device(dev);
    kfree(uio);
}

static struct pci_device_id connector_pci_ids[] = {
    {
        .vendor =       PCI_VENDOR_ID_REDHAT_QUMRANET,
        .device =       PCI_CONNECTOR_DEVICE_ID,
        .subvendor =    PCI_ANY_ID,
        .subdevice =    PCI_ANY_ID,
    },
    {0,}
};

static struct pci_driver connector_pci_driver = {
    .name = "connector",
    .id_table = connector_pci_ids,
    .probe = connector_pci_probe,
    .remove = connector_pci_remove,
};


static int __init connector_init_module (void)
{
    int result = pci_register_driver(&connector_pci_driver);

    printk("Measurement Module Start\n");

    send_ready_signal();

    /*
    dataportWait();

    u32* dpData = kmalloc(4096, GFP_KERNEL);

    dataportRead(dpData);

    dataportPrintData();

    // build string to write
    u32* dpNewData = kmalloc(4096, GFP_KERNEL);
    int i;
    for (i=0; i<1024; i++)
    {
        dpNewData[i] = 0x6d6d6d6d;
    }

    dataportWrite(dpNewData, 1024);

    dataportRead(dpData);

    dataportPrintData();
    */

    return result;
}

static void __exit connector_exit_module(void)
{
    pci_dev_put(devices[0]->dev);
}



module_init(connector_init_module);
module_exit(connector_exit_module);

//module_pci_driver(connector_pci_driver);

MODULE_DEVICE_TABLE(pci, connector_pci_ids);
MODULE_LICENSE("GPL v2");

