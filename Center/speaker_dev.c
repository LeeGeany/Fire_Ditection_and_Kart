#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define SPEAKER_MAJOR_NUMBER 503
#define SPEAKER_DEV_NAME	"speaker_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0x04
#define GPSET0	0x1C
#define GPCLR0	0x28
#define GPLEV0	0x34

static char *buffer=NULL;


#define IOCTL_MAGIC_NUMBER	'j'
#define IOCTL_CMD_SET_DIRECTION			_IOWR(IOCTL_MAGIC_NUMBER,0,int)
#define IOCTL_CMD_TOGGLE				_IOWR(IOCTL_MAGIC_NUMBER,1,int)
#define IOCTL_CMD_LEVEL					_IOWR(IOCTL_MAGIC_NUMBER,2,int)


static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;
volatile unsigned int *gplev1;



int speaker_dev_open(struct inode *innode, struct file *filp){
printk(KERN_ALERT "speaker_dev open function called\n");

gpio_base = ioremap(GPIO_BASE_ADDR,0x60);

gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
gpset1 = (volatile unsigned int *)(gpio_base + GPSET0);
gpclr1 = (volatile unsigned int *)(gpio_base + GPCLR0);
gplev1 = (volatile unsigned int *)(gpio_base + GPLEV0);


return 0;
}

int speaker_dev_release(struct inode *inode, struct file *filp){
printk(KERN_ALERT "speaker_dev release function called\n");
iounmap((void*)gpio_base);
return 0;
}


long speaker_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
int kbuf=-1;


switch(cmd){
	case IOCTL_CMD_SET_DIRECTION:
		copy_from_user(&kbuf,(const void*)arg,4);
		if(kbuf==0){
			//set direction in
			printk(KERN_ALERT "SPEAKER set direction in!!\n");
			*gpsel1 |=(0<<24);
		}else if(kbuf ==1){
			printk(KERN_ALERT "SPEAKER set direction out!!\n");
			*gpsel1 |= (1<<24);
		}else{
			//error
			printk(KERN_ALERT "Error direction parameter error\n");
			return -1;
		}
		break;
		
	case IOCTL_CMD_TOGGLE:
		copy_from_user(&kbuf,(const void*)arg,4);
		if(kbuf==0){
			printk(KERN_ALERT "sound on!!\n");
			*gpset1 |= (1<<18);
			ssleep(1);
			}
			else{
			printk(KERN_ALERT "sound off!!\n");
			*gpclr1 |= (1<<18);
			ssleep(1);
		}
		break;
		
	case IOCTL_CMD_LEVEL:
		if((*gplev1 && (1<<18))!=0){
			return 1;    // on
		}
		else{
			return 2;	// off
		}
		break;
	}
	return 0;
}

static struct file_operations sys_fops = {
.owner = THIS_MODULE,
.unlocked_ioctl = speaker_ioctl,
.open = speaker_dev_open,
.release = speaker_dev_release
};



int __init speaker_dev_init(void){
if(register_chrdev(SPEAKER_MAJOR_NUMBER	, "speaker_dev",&sys_fops)<0)
	printk(KERN_ALERT "[speaker_dev] driver init failed\n");
else
	printk(KERN_ALERT "[speaker_dev] driver init successful\n");
buffer = (char *)kmalloc(1024,GFP_KERNEL);
if(buffer !=NULL)
	memset(buffer,0,1024);
return 0;
}



void __exit speaker_dev_exit(void){
unregister_chrdev(SPEAKER_MAJOR_NUMBER	,"speaker_dev");
printk(KERN_ALERT "[speaker_dev] driver cleanup\n");
kfree(buffer);
}

module_init(speaker_dev_init);
module_exit(speaker_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("taek");
MODULE_DESCRIPTION("speaker_dev");
