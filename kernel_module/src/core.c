//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng, Yu-Chia Liu
//
//   Description:
//     Core of Kernel Module for Processor Container
//
////////////////////////////////////////////////////////////////////////

#include "processor_container.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/sched.h>

struct container_list
{
    __u64 cid;
    struct thread_list* head;
    struct thread_list* cur;
    struct container_list* next;
};

struct thread_list
{
    struct task_struct* thread;
    struct thread_list* next;
};

extern struct miscdevice processor_container_dev;

struct container_list* start;
struct mutex container_mutex;

/**
 * Initialize and register the kernel module
 */
int processor_container_init(void)
{
    int ret;
    if ((ret = misc_register(&processor_container_dev)))
        printk(KERN_ERR "Unable to register \"processor_container\" misc device\n");
    else
        printk(KERN_ERR "\"processor_container\" misc device installed\n");
        mutex_init(&container_mutex);
        start = NULL;
        // printk(KERN_INFO "Hello world %lu..\n", sizeof(*container_list));
    return ret;
}


/**
 * Cleanup and deregister the kernel module
 */ 
void processor_container_exit(void)
{
    misc_deregister(&processor_container_dev);
}
