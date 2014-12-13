#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/export.h>
MODULE_LICENSE("GPL");

extern int cache_on;
//extern int timing_on;
static int __init cache_init(void)
{
	cache_on=1;
	//timing_on=1;
	printk("cache on.\n");	
	return 0;

}

static void __exit cache_exit(void)
{
	printk("cache off.\n");

}

module_init(cache_init);
module_exit(cache_exit);
