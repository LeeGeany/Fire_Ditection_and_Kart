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

#define MOTER_MAJOR_NUMBER  	 501
#define MOTER_DEV_NAME          "dcmoter"    

#define GPIO_BASE_ADDR		0x3F200000
#define GPFSEL1			0x04
#define GPSET0			0x1C
#define GPCLR0			0x28

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;


void wheel_forward(void){
    *gpclr1 |= (1<<18);
    *gpset1 |= (1<<17);
    *gpclr1 |= (1<<16);

    *gpset1 |= (1<<18);
}

void wheel_backward(void){
    *gpclr1 |= (1<<18);
    *gpclr1 |= (1<<17);
    *gpset1 |= (1<<16);

    *gpset1 |= (1<<18);
}

void wheel_stop(void){
    *gpclr1 |= (1<<18);
    *gpclr1 |= (1<<17);
    *gpclr1 |= (1<<16);   
    
    *gpset1 |= (1<<18);
}

int moter_open(struct inode *inode , struct file* flip){
    printk(KERN_ALERT "Moter Driver Open\n");
	
    gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
    gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
    gpset1 = (volatile unsigned int *)(gpio_base + GPSET0);
    gpclr1 = (volatile unsigned int *)(gpio_base + GPCLR0);

    /* right wheel control */
    *gpsel1 |= (1<<24);             // gpio 18 P_BRG_EN12
    *gpsel1 |= (1<<21);             // gpio 17 P_BRG_1A
    *gpsel1 |= (1<<18);             // gpio 16 P_BRG_2A

    return 0;
}

int moter_release(struct inode *inode ,struct file *flip){
    printk(KERN_ALERT "Moter Driver Release\n");
    iounmap((void*)gpio_base);
    return 0;
}

ssize_t moter_write(struct file*flip, const char *buf, size_t count, loff_t *fops){
    printk(KERN_ALERT "Moter Device Read\n");
    int select =0;
    copy_from_user(&select, buf, 4);
    
    // FORWARDING...
    if(select == 1){
	printk(KERN_ALERT "RIGHT WHEEL FORWARDING...\n");
	wheel_forward(); 
    }
    // BACKWARDING...
    else if(select == 2){
	printk(KERN_ALERT "RIGHT WHEEL BACKWARDING...\n");
	wheel_backward(); 
    }
    // STOP...
    else if(select == 3){
	printk(KERN_ALERT "RIGHT WHEEL STOPPING...\n");
	wheel_stop(); 	
    }

    return count;
}

static struct file_operations sys_fops = {
    .owner = THIS_MODULE,
    .write = moter_write,
    .open = moter_open,
    .release = moter_release
};

int __init moter_driver_init(void){
    if(register_chrdev(MOTER_MAJOR_NUMBER,MOTER_DEV_NAME, &sys_fops) < 0){
		printk(KERN_ALERT "[SYSPROG] DRIVER INIT FAILED\n");
	}
	else{
		printk(KERN_ALERT "[SYSPROG] DRIVER INIT SUCCESS\n");
	}
	return 0;
}

void __init moter_driver_exit(void){
	unregister_chrdev(MOTER_MAJOR_NUMBER, MOTER_DEV_NAME);
	printk(KERN_ALERT "[SYSPROG] DRIVER CLEANUP\n");
}

module_init(moter_driver_init);
module_exit(moter_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LEE JINHEE");
MODULE_DESCRIPTION("MOTER DRIVER");
