#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <asm/kvm_para.h>
#include <asm/io.h>

static int __init good_init(void) {
    printk("==============================\n");
    printk("Good Module Init");
    printk("\n==============================\n");
    return 0;
}

static void __exit good_exit(void) {
    printk("\n==============================\n");
    printk("Good Module Exit");
    printk("\n==============================\n");
}

module_init(good_init);
module_exit(good_exit);
