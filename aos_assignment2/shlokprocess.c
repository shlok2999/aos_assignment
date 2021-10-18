#include <linux/kernel.h>

#include <linux/sched.h>


asmlinkage long shlokprocess(void)
{
	printk("CID:%d  PID:%d",current->pid,task_ppid_nr(current));
	//printk("Parent Process id:%d",task_ppid_nr(current));
	return 0;
}
