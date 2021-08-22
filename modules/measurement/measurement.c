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
#include "sha.c"

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

u8* signoff = (u8*)"DEADBEEF";

enum MeasurementState
{
    Initing,
    Waiting,
    CollectingModuleMeasurements,
    SendingModuleMeasurements,
    CollectingLinuxMeasurements,
    SendingLinuxMeasurements
};

struct ModuleMeasurement
{
    uint8_t name[256];
    uint8_t* rodata;
    uint32_t rosize;
};

typedef struct MeasurementManager
{
    enum MeasurementState state;
    struct ModuleMeasurement** measurements;
    uint8_t numMeasurements;
    uint8_t** payloads;
    uint8_t numPayloads;
} MeasurementManager;
MeasurementManager measurementManager = { Waiting, NULL, 0, 0, 0 };

static void resetMeasurementManager(void)
{
    int i;
    for(i=0; i<measurementManager.numMeasurements; i++)
    {
        kfree(measurementManager.measurements[i]->rodata);
        measurementManager.measurements[i]->rodata = NULL;
        kfree(measurementManager.measurements[i]);
        measurementManager.measurements[i] = NULL;
    }
    for(i=0; i<measurementManager.numPayloads; i++)
    {
        kfree(measurementManager.payloads[i]);
        measurementManager.payloads[i] = NULL;
    }
    kfree(measurementManager.measurements);
    measurementManager.measurements = NULL;
    kfree(measurementManager.payloads);
    measurementManager.payloads = NULL;
}

/*
** A 4096-byte payload is a sequence of 46 module measurements
** each measurement is 88 bytes
** - 56 bytes for the module name (as defined as MODULE_NAME_LEN in module.h)
** - 32 bytes for a SHA256 hash digest
*/
static void buildModulePayloads(void)
{
    int i;
    int j;
    int k;
    measurementManager.numPayloads = 0;
    // can have 1 payload = 46 modules
    measurementManager.payloads = kzalloc(1 * sizeof(uint8_t*), GFP_KERNEL);

    // we can put 46 hashed measurements into each payload
    while(measurementManager.numPayloads*46 < measurementManager.numMeasurements)
    {
        measurementManager.payloads[measurementManager.numPayloads] = kzalloc(4096, GFP_KERNEL);
        measurementManager.numPayloads++;
    }

    for(i=0; i<measurementManager.numPayloads; i++)
    {
        for(j=0; j<46; j++)
        {
            unsigned char *digest;
            struct ModuleMeasurement* thisMsmt = measurementManager.measurements[46*i + j];
            
            // sign off if necessary
            if(thisMsmt == NULL)
            {
                printk("Built all measurements. Add sign-off.\n");
                printk("Module name len is %lu\n", MODULE_NAME_LEN);
                strcpy(measurementManager.payloads[i] + 88*j, signoff);
                break;
            }

            printk("Computing hash for %s module\n", thisMsmt->name);
            // compute the hash digest of the rodata
            digest = kzalloc(32, GFP_KERNEL);
            if(digest < 0)
            {
                printk("Digest Malloc Failed.\n");
            }

            if(strcmp(thisMsmt->name, "measurement")==0)
            {
                do_sha256(thisMsmt->rodata, 0, digest);
            }
            else
            {
                do_sha256(thisMsmt->rodata, thisMsmt->rosize, digest);
            }

            // load the measurement into the payloads array
            strcpy(measurementManager.payloads[i] + 88*j, thisMsmt->name);
            memcpy(measurementManager.payloads[i] + 88*j + 56, digest, 32);
            kfree(digest);
        }
    }
}


/*
static void dataportRead(u32* result)
{
    int i;
    void* internal_addr = devices[0]->uio->mem[1].internal_addr;
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
    u8* inputBytes;
    u32* input = kmalloc(4096, GFP_KERNEL);
    dataportRead(input);

    // print 8 rows of 128
    printk("Dataport Contents:\n");
    inputBytes = (u8*)input;
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
*/

static void send_ready_signal(void)
{
    uint32_t* event_bar = devices[0]->uio->mem[0].internal_addr;
    writel(1, &event_bar[0]);
}

static void dataportWrite(uint8_t* input, int length)
{
    int i;
    void* internal_addr;
    if(4096 < length)
    {
        printk("dataportWrite: length too large: input truncated");
    }
    internal_addr = devices[0]->uio->mem[1].internal_addr;
    for (i=0; i<length; i++)
    {
        writeb(input[i], internal_addr);
        internal_addr = (u8*)internal_addr + 1;
    }
}

// size in bytes
static void sendNextPayload(void)
{
    int i;
    for(i=0; i<measurementManager.numPayloads; i++)
    {
        printk("Load Payload %d\n", i);
        dataportWrite(measurementManager.payloads[i], 4096);
        printk("Send Ready Signal\n");
        send_ready_signal();
    }
    printk("Transaction Complete\n");
    measurementManager.state = Waiting;
    resetMeasurementManager();
}

static void measureModules(void)
{
    struct module *mod;
    int it = 0;

    /*
    ** Assume there can be at most 100 modules
    ** Assume no module has a name longer than 256 characters
    ** :shrug:
    */
    measurementManager.measurements = kzalloc(100 * sizeof(struct ModuleMeasurement*), GFP_KERNEL);
    measurementManager.numMeasurements = 0;

//   mutex_lock(&module_mutex);
    //printk("my name is: %s\n", &THIS_MODULE->name);
    list_for_each_entry(mod, &THIS_MODULE->list, list)
    {
        char* thisName;
        struct ModuleMeasurement* msmt = kzalloc(sizeof(struct ModuleMeasurement), GFP_KERNEL);
        uint8_t* rodataPtr = (uint8_t*)mod->core_layout.base;
        u8 firstByte = ((u8*)mod->name)[0];
        if( 0x20 <= firstByte && firstByte <= 0x7F )
        {
            // copy away these module data
            thisName = mod->name;
            msmt->rosize = mod->core_layout.ro_size;
            msmt->rodata = kzalloc(msmt->rosize, GFP_KERNEL);
            memcpy(msmt->rodata, rodataPtr, msmt->rosize);
            //printk("size: %d\ntext_size %d\nro_size %d\n", mod->core_layout.size, mod->core_layout.text_size,mod->core_layout.ro_size);
        }
        else
        {
            /*
            ** these data are NOT available for &THIS_MODULE
            ** if we want to report on this module,
            ** we'll have to find something else to hash
            */
            thisName = "measurement";
            msmt->rodata = NULL;
            msmt->rosize = 0;
        }

        strcpy(msmt->name, thisName);

        measurementManager.measurements[measurementManager.numMeasurements] = msmt;
        measurementManager.numMeasurements++;
    }
//    mutex_unlock(&module_mutex);

    measurementManager.state = SendingModuleMeasurements;
    buildModulePayloads();
    sendNextPayload();
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
        case Initing:
            printk("Still initializing...\n");
        case Waiting:
            printk("Got a Measurement Request\n");
            measurementManager.state = CollectingModuleMeasurements;
            measureModules();
            break;
        case CollectingModuleMeasurements:
            printk("Still measuring...\n");
            break;
        case SendingModuleMeasurements:
            sendNextPayload();
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

    printk("Measurement Module Ready\n");

    measurementManager.state = Waiting;
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

