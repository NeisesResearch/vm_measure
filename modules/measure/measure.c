#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <asm/kvm_para.h>
#include <asm/io.h>

//includes for emit/consume events
//#include <stdio.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <assert.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/mman.h>


struct buffer {
    char* p;
    int pos;
    int size;
};

/*
static void wait(void)
{
    int fd = open("/dev/uio0", O_RDWR);
    int val;
    int result = read(fd, &val, sizeof(val));
    if(result < 0)
    {
        printk("Error: %s\n", strerror(errno));
    }
    else
    {
        printk("Back from waiting with val: %d\n", val);
    }
    close(fd);
}

static void emit(void)
{
    int fd = open("/dev/uio0", O_RDWR);
    assert(fd >= 0);

    char* connection;
    if ((connection = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 * getpagesize())) == (void*) -1)
    {
        printk("mmap failed\n");
        close(fd);
    }

    // emit signal
    connection[0] = 1;

    munmap(connection, 0x1000);
    close(fd);
}

// must allocate dataport yourself
static void read(char* dataport)
{
    int fd = open("/dev/uio0", O_RDWR);
    assert(fd >= 0);

    int length = 4096;
    if ((dataport = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 * getpagesize())) == (void*) -1)
    {
        printk("mmap failed\n");
    }
    close(fd);
}

static void write(char* message)
{
    int fd = open("/dev/uio0", O_RDWR);
    assert(fd >= 0);

    char* dataport;
    int length = 4096;
    if ((dataport = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 * getpagesize())) == (void*) -1)
    {
        printk("mmap failed\n");
        close(fd);
    }

    int i=0;
    char ch;
    while ((ch = getchar()) != EOF && i < length-1)
    {
        dataport[i] = ch;
        i++;
    }
    close(fd);

}
*/


static int __init hello_init(void)
{
    /*
    while(1)
    {
        wait();
        printk("got an event!\n");

        struct module *mod;
        struct module_layout myLayout;
        int it = 0;

        printk("\n==============================\n");
        printk("Listing the loaded kernel modules...\n");

    //   mutex_lock(&module_mutex);
        list_for_each_entry(mod, &THIS_MODULE->list, list)
                printk(KERN_INFO "%s\n", mod->name);
                myLayout = mod->core_layout;
                unsigned long* thisBase = (unsigned long*)myLayout.base;

                for(it=0; it<myLayout.size / sizeof(unsigned long); it++)
                {
                    printk("%lu\n", thisBase[it]);
                }
    //    mutex_unlock(&module_mutex);


        printk("\n==============================\n");

        emit();
    }
    */

    return 0;
}

static void __exit hello_exit(void) {
    printk("\n==============================\n");
    printk("Bye-Bye");
    printk("\n==============================\n");
}

module_init(hello_init);
module_exit(hello_exit);



