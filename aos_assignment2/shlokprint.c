#include<linux/kernel.h>
#include<linux/syscalls.h>

SYSCALL_DEFINE1(shlokprint,char*,msg)
{
	char buff[256];
	long val=strncpy_from_user(buff, msg , sizeof(buff));
	if(val < 0 || val==sizeof(buff))
		return -EFAULT;
	
	printk("%s",buff);
	return 0;
}
