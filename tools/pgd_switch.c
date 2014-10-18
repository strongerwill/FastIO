#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/export.h>
MODULE_LICENSE("GPL");

extern int cache_on;
static int __init cache_init(void)
{
	cache_on=1;
	printk("cache switch setup.\n");	
	return 0;

}

static void __exit cache_exit(void)
{
	printk("cache switch clean.\n");

}

module_init(cache_init);
module_exit(cache_exit);
