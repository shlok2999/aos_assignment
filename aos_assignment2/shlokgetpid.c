#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/sched.h>


asmlinkage long shlokgetpid(void)
{
	return current->pid;
}
