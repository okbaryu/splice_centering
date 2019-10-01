#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>

#include "offs_io.h"

extern int mmc_offs_read(u8 pdrv, u8 *buff, u32 sector, u32 count);
extern int mmc_offs_write(u8 pdrv, const u8 *buff, u32 sector, u32 count);

#define MAX_BUFFER_SIZE	(64*1024)
static u8 file_buffer[MAX_BUFFER_SIZE];

static DEFINE_MUTEX(offs_lock);

static long offs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;


	mutex_lock(&offs_lock);

	//printk(">>0x%08x\n", cmd);

	switch (cmd) {
	case OFFS_IO_WRITE:
	{

		struct offs_data data;


		memset(&data, 0, sizeof(struct offs_data ));
		if(copy_from_user(&data, (void __user *)arg, sizeof(struct offs_data))) {
			ret =  -EFAULT;
			break;
		}

		if(data.count == 0) {
			ret = 0;
			break;
		}

		if(data.count*512 > MAX_BUFFER_SIZE)
			data.count = MAX_BUFFER_SIZE/512;

		if(copy_from_user(file_buffer, data.buffer, data.count*512) != 0) {
			ret =  -EFAULT;
			break;
		}

		if(mmc_offs_write(0, file_buffer, data.sector, data.count) != 0) {
			ret = -EFAULT;
			break;
		}

		ret = data.count;
		break;
	}
	case OFFS_IO_READ:
	{
		struct offs_data data;
		
		memset(&data, 0, sizeof(struct offs_data ));
		if(copy_from_user(&data, (void __user *)arg, sizeof(struct offs_data))) {
			ret =  -EFAULT;
			break;
		}

		if(data.count == 0) {
			ret = 0;
			break;
		}

		if(data.count*512 > MAX_BUFFER_SIZE)
			data.count = MAX_BUFFER_SIZE/512;

		if(mmc_offs_read(0, file_buffer, data.sector, data.count) != 0) {
			ret = -EFAULT;
			break;
		}

		if(copy_to_user(data.buffer, file_buffer, data.count*512) != 0) {
			ret =  -EFAULT;
			break;
		}

		ret = data.count;
		break;
	}
	case OFFS_IO_GET_SECTOR_COUNT:
	{
		u32 mmc_offs_sectorcount(void);
		u32 sector_count = mmc_offs_sectorcount();
		if(copy_to_user((void __user *)arg, &sector_count, sizeof(u32)) != 0) {
			ret =  -EFAULT;
			break;
		}
	}
	default:
		break;
	}

	//printk("<<0x%08x\n", cmd);

	mutex_unlock(&offs_lock);
	
	return ret;
}

static int opend_times = 0;

static int offs_open(struct inode *inode, struct file *filp)
{
	struct task_struct * process = current;

	printk ("[offs.ko] open by pid [%d]\n", process->pid);

	mutex_lock(&offs_lock);

	if (opend_times == 0) {
	}

	opend_times++;

	mutex_unlock(&offs_lock);

	return 0;
}


static int offs_release(struct inode *inode, struct file *filp)
{
	struct task_struct * process = current;

	printk ("[offs.ko] release by pid [%d]\n", process->pid);

	mutex_lock(&offs_lock);

	opend_times--;


	mutex_unlock(&offs_lock);

	return 0;
}

static ssize_t offs_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	printk ("[offs.ko] read\n");

	return -1;
}

static struct file_operations offs_fops =
{
	.owner   = THIS_MODULE,
	.read    = offs_read,
	.unlocked_ioctl = offs_ioctl,
	.open    = offs_open,
	.release = offs_release
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "offs",
	.fops = &offs_fops,
};

static int __init dev_init(void)
{
	int ret = misc_register(&misc);

	printk ("offs initialized\n");

	return ret;
}

static void __exit dev_exit(void)
{
	misc_deregister(&misc);
	printk("offs unloaded\n");
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("coben.han");
