#!/bin/bash

# First cleanup existing device
rm -rf /dev/gpiodriver

# Load the kernel module
insmod gpio_driver.ko

# Parse major driver number from /proc/devices output
drv_major=$(awk "/gpiodriver/ {print \$1}" /proc/devices)

mknod /dev/gpiodriver c $drv_major 0

sudo chmod o+rw /dev/gpiodriver
