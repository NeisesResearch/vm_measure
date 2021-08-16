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

static int __init hello_init(void)
{
    struct module *mod;

    printk("\n==============================\n");
    printk("Listing the loaded kernel modules...\n");

//   mutex_lock(&module_mutex);
    list_for_each_entry(mod, &THIS_MODULE->list, list)
    {
        // check range of first char
        u8* u8Ptr = (u8*)mod->name;
        u8 firstByte = u8Ptr[0];
        if( 0x20 <= firstByte && firstByte <= 0x7F)
        {
            /*
            for(it=0; it<32; it++)
            {
                printk("%x", u8Ptr[it]);
            }
            printk("\n");
            */
            printk("%s\n", mod->name);
        }
        else
        {
            printk("Found self\n");
        }

    }
    /*
            myLayout = mod->core_layout;
            unsigned long* thisBase = (unsigned long*)myLayout.base;

            for(it=0; it<myLayout.size / sizeof(unsigned long); it++)
            {
                printk("%lu\n", thisBase[it]);
            }
            */
//    mutex_unlock(&module_mutex);


    printk("\n==============================\n");

    return 0;
}

static void __exit hello_exit(void) {
    printk("\n==============================\n");
    printk("Bye-Bye");
    printk("\n==============================\n");
}

module_init(hello_init);
module_exit(hello_exit);



