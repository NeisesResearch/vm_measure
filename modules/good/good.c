// michael neises
// work queue tests
// https://tuxthink.blogspot.com/2011/09/workqueue-2-declarework.html
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

void workq_fn(struct work_struct*);
DECLARE_WORK(workq,workq_fn);
void workq_fn(struct work_struct* work)
{
    long c;
    atomic_long_set(&(workq.data),10); // I don't know why or whether its important to set this .data field
    printk("beep beep: %d\n", atomic_long_read(&(workq.data)));
}

ssize_t sched_workq(struct file* buf, char* start, size_t offset, loff_t* len)
{
    printk(KERN_INFO "In proc sched workq");
    schedule_work(&workq);
    return 0;
}
int sched_workq_old(char *buf, char** start, off_t offset, int len, int* eof, void* arg)
{
    printk(KERN_INFO "In proc sched workq");
    schedule_work(&workq);
    return 0;
}

static const struct file_operations proc_file_fops = {
    .owner = THIS_MODULE,
    //.open = open_callback,
    .read = sched_workq,
};
int st_init(void)
{
    printk("==============================\n");
    printk("Good Module Init\n");
    printk("Create /proc/sched_workq\n");
    printk("Try `cat` on it.\n");
    printk("==============================\n");
    proc_create("sched_workq", 0, NULL, &proc_file_fops);
    return 0;
}

void st_exit(void)
{
    remove_proc_entry("sched_workq", NULL);
}

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

/*
module_init(good_init);
module_exit(good_exit);
*/
module_init(st_init);
module_exit(st_exit);
