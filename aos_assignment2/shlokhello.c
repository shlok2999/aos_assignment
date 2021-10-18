#include <linux/kernel.h>

asmlinkage long shlokhello(void)
{
	printk("Welcome to Linux shlok");
	return 0;
}
