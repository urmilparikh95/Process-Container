#!/bin/bash

dmesg -w &
cd kernel_module/
sudo rmmod processor_container
make
sudo make install
sudo insmod processor_container.ko
sudo chmod 777 /dev/pcontainer