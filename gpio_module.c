#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/io.h>

#define MAX_USER_SIZE 1024

#define BCM2837_GPIO_ADDRESS 0x3F200000
#define BCM2711_GPIO_ADDRESS 0xfe200000

static struct proc_dir_entry *proc = NULL;

static char data_buffer[MAX_USER_SIZE + 1] = {0};

static unsigned int *gpio_registers = NULL;
static unsigned int gpio_selected = 22;

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

ssize_t read(struct file *file, char __user *user, size_t size, loff_t *off)
{
    printk("Inside read");
    int r = read_gpio(gpio_selected);
    static char buffer[5];
    snprintf(buffer, sizeof(buffer), "%d", r);
    return copy_to_user(user, buffer, 1) ? 0 : 1; // copia datos del espacio del kernel al espacio de usuario
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

    printk("Data buffer: %s\n", data_buffer);

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

static const struct proc_ops proc_fops = {
        .proc_read = read,
        .proc_write = write,
};

static int __init gpio_module_init(void) {
    gpio_registers = (int *)ioremap(BCM2837_GPIO_ADDRESS, PAGE_SIZE);
    if (gpio_registers == NULL) {
        printk("Failed to map GPIO memory to driver\n");
        return -1;
    }


    printk("Successfully mapped in GPIO memory\n");

    // create an entry in the proc-fs
    proc = proc_create("gpio", 0666, NULL, &proc_fops);
    if (proc == NULL) {
        return -1;
    }

    printk("river loaded successfully!\n");

    return 0;
}

static void __exit gpio_module_exit(void)
{
    printk("Leaving my driver!\n");
    iounmap(gpio_registers);
    proc_remove(proc);
    return;
}

module_init(gpio_module_init);
module_exit(gpio_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SistemOfACom");
MODULE_DESCRIPTION("This module reads two gpio pins from the RASPI");
MODULE_VERSION("1.0");
