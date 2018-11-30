#!/bin/bash
sudo insmod obj/altera_driver.ko
sudo mknod /dev/de2i150_altera c 91 1