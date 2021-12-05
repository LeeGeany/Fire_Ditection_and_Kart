
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define LED_MAJOR_NUMBER  	 201
#define LED_DEV_NAME          "led"    

#define GPIO_BASE_ADDR		0x3F200000
#define GPFSEL1			0x00
#define GPSET0			0x1C
#define GPCLR0			0x28

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;

int led_open(struct inode *inode , struct file* flip){
    printk(KERN_ALERT "Moter Driver Open\n");
	
    gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
    gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
    gpset1 = (volatile unsigned int *)(gpio_base + GPSET0);
    gpclr1 = (volatile unsigned int *)(gpio_base + GPCLR0);
	*gpsel1 |= (1 << 12);
	*gpclr1 |= (1 << 4 );
    return 0;
}

int led_release(struct inode *inode ,struct file *flip){
    iounmap((void*)gpio_base);
    return 0;
}

ssize_t led_write(struct file*flip, const char *buf, size_t count, loff_t *fops){
    int state =0;
    printk(KERN_ALERT "MOTER WRITE STATE = %d\n", state);
    copy_from_user(&state, buf, 4);
    if(state == 1){ *gpset1 |= (1 << 4 );}
    else if(state == 0){ *gpclr1 |= ( 1 << 4 ); }
    return count;
}

static struct file_operations sys_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
    .open = led_open,
    .release = led_release
};

int __init moter_driver_init(void){
    if(register_chrdev(LED_MAJOR_NUMBER,LED_DEV_NAME, &sys_fops) < 0){
		printk(KERN_ALERT "[SYSPROG] DRIVER INIT FAILED\n");
	}
	else{
		printk(KERN_ALERT "[SYSPROG] DRIVER INIT SUCCESS\n");
	}
	return 0;
}

void __init moter_driver_exit(void){
	unregister_chrdev(LED_MAJOR_NUMBER, LED_DEV_NAME);
	printk(KERN_ALERT "[SYSPROG] DRIVER CLEANUP\n");
}

module_init(moter_driver_init);
module_exit(moter_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LEE JINHEE");
MODULE_DESCRIPTION("MOTER DRIVER");
