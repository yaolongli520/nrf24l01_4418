#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/spi/spi.h> 
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/wait.h> 
#include <linux/mutex.h>

static pid_t simple_pid;
static wait_queue_head_t wq;
static int t_flag; 

int threadfunc(void *data)
{
	printk("%s %d \n",__func__,__LINE__);
//	t_flag = 1;
//	wake_up(&wq);
	return 0;
}



static int __init test_work_init(void)
{
	init_waitqueue_head(&wq);
	simple_pid =  kernel_thread(threadfunc,NULL,SIGCHLD);
	
	t_flag = 1;
	#if 0
	if(!wait_event_timeout(wq,t_flag,HZ*2))
	{
		printk("wait is timeout \n");
		
	}
	#endif //睡眠前 t_flag = 1 是否不进入睡眠？是的
	wait_event_interruptible(wq,t_flag);
	
	printk("%s %d \n",__func__,__LINE__);
	return 0;
}

static void __exit test_work_exit(void)
{
	printk("%s %d \n",__func__,__LINE__);
}





module_exit(test_work_exit);
module_init(test_work_init);


MODULE_AUTHOR("topeet: rty");
MODULE_LICENSE("GPL");











