#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <asm/atomic.h>

#define MAX_USER_SIZE 1024

#define BCM2837_GPIO_ADDRESS 0x3F200000

#define DEV_NAME "gpiodriver"


static char data_buffer[MAX_USER_SIZE + 1] = {0};

static unsigned int *gpio_registers = NULL;
static unsigned int gpio_selected = 22;

struct cdev *gpio_cdev;
int drv_major = 0;

static void gpio_pin_setup(unsigned int pin) {
    unsigned int fsel_index = pin / 10;
    unsigned int fsel_bitpos = pin % 10;
    unsigned int *gpio_fsel = gpio_registers + fsel_index;

    *gpio_fsel &= ~(7 << (fsel_bitpos * 3));
    gpio_selected = pin;

    return;
}

static int read_gpio(unsigned int pin) {
    unsigned int lev_index = pin / 32;
    unsigned int lev_bitpos = pin % 32;
    volatile unsigned int* gpio_lev = (volatile unsigned int*)((char*)gpio_registers + 0x34 + (lev_index * 4));

    return (*gpio_lev & (1 << lev_bitpos)) ? 1 : 0;
}

ssize_t read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
    if(*f_pos > 0) {
        return 0; //EOF
    }

    int r = read_gpio(gpio_selected);
    udelay(1000);
    if(copy_to_user(buf, &r, sizeof(r)))
    {
        return -EFAULT;
    
    }
    return sizeof(r);
}

ssize_t write(struct file *file, const char __user *user, size_t size, loff_t *off)
{
    unsigned int pin = 22;

    memset(data_buffer, 0x0, sizeof(data_buffer));

    if (size > MAX_USER_SIZE) {
        size = MAX_USER_SIZE;
    }

    if (copy_from_user(data_buffer, user, size))
        return 0;

    if (sscanf(data_buffer, "%d", &pin) != 1) {
        printk("Inproper data format submitted\n");
        return size;
    }

    if (pin > 21 || pin < 0) {
        printk("Invalid pin number submitted\n");
        return size;
    }

    printk("Pin %d selected\n", pin);
    gpio_pin_setup(pin);


    return size;
}

struct file_operations gpio_fops = {
	.owner =     THIS_MODULE,
	.read =	     read,
    .write =     write
};

static int __init gpio_module_init(void) {

    int result;
    dev_t dev = MKDEV(drv_major,0);

    printk("Initializing GPIO driver\n");
    result = alloc_chrdev_region(&dev, 0, 1, DEV_NAME);
    drv_major = MAJOR(dev);

    if (result < 0) {
        pr_alert("[GPIO]: Error in alloc_chrdev_region\n");
        return result;
    }

    gpio_cdev = cdev_alloc();
    gpio_cdev->ops = &gpio_fops;
    result = cdev_add(gpio_cdev, dev, 1);
    if (result < 0) {
        printk("[GPIO]: Error in cdev_add\n");
        unregister_chrdev_region(dev, 1);
        return result;
    }

    gpio_registers = (int *)ioremap(BCM2837_GPIO_ADDRESS, PAGE_SIZE);
    if (gpio_registers == NULL) {
        printk("Failed to map GPIO memory in Raspi to driver\n");
        return -1;
    }

    printk("Successfully loaded GPIO driver\n");

    return 0;
}

static void __exit gpio_module_exit(void)
{
    printk("Removing GPIO driver\n");
    dev_t dev = MKDEV(drv_major,0);
    cdev_del(gpio_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(gpio_module_init);
module_exit(gpio_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SistemOfACom");
MODULE_DESCRIPTION("This module reads two GPIO pins from the RASPI");
MODULE_VERSION("1.0");
