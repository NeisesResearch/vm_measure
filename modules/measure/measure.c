#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <asm/kvm_para.h>
#include <asm/io.h>


struct buffer {
    char* p;
    int pos;
    int size;
};

static int __init hello_init(void) {
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
    return 0;
}

static void __exit hello_exit(void) {
    printk("\n==============================\n");
    printk("Bye-Bye");
    printk("\n==============================\n");
}

module_init(hello_init);
module_exit(hello_exit);



