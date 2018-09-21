#!/bin/bash

#dmesg -w &
sudo rmmod processor_container
cd kernel_module/
make
sudo make install
sudo insmod processor_container.ko
sudo chmod 777 /dev/pcontainer