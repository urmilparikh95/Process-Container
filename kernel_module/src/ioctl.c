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
#include <linux/kthread.h>

struct container_list
{
    __u64 cid;
    struct thread_list* head;
    struct thread_list* current;
    struct container_list* next;
};

struct thread_list
{
    struct task_struct* thread;
    struct thread_list* next;
};

extern struct mutex container_mutex;

struct container_list* h = NULL;

/**
 * Delete the task in the container.
 * 
 * external functions needed:
 * mutex_lock(), mutex_unlock(), wake_up_process(), 
 */
int processor_container_delete(struct processor_container_cmd __user *user_cmd)
{
    // struct processor_container_cmd *kernel_cmd;
    // kernel_cmd = kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    // copy_from_user(kernel_cmd, user_cmd, sizeof(*user_cmd));
    // printk(KERN_INFO "Hello world %llu..\n", kernel_cmd->cid);
    return 0;
}

/**
 * Create a task in the corresponding container.
 * external functions needed:
 * copy_from_user(), mutex_lock(), mutex_unlock(), set_current_state(), schedule()
 * 
 * external variables needed:
 * struct task_struct* current  
 */
int processor_container_create(struct processor_container_cmd __user *user_cmd)
{
    struct processor_container_cmd *kernel_cmd;
    kernel_cmd = kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    copy_from_user(kernel_cmd, user_cmd, sizeof(*user_cmd));
    struct thread_list* temp_thread = (struct thread_list*)kmalloc(sizeof(struct thread_list), GFP_KERNEL);
    temp_thread->thread = current;
    temp_thread->next = NULL;
    if(h == NULL)
    {
        struct container_list* temp_container = (struct container_list*)kmalloc(sizeof(struct container_list), GFP_KERNEL);
        temp_container->cid = kernel_cmd->cid;
        temp_container->head = temp_thread;
        temp_container->current = temp_container->head;
        temp_container->next = NULL;
        h = temp_container;
    }
    else
    {
        int flag = 0;
        struct container_list* c = h;
        while(c != NULL)
        {
            if(c->cid == kernel_cmd->cid)
            {
                /// 1 head always present
                flag = 1;
                break;
            }
            c = c->next;
        }
        if(flag == 0)
        {
            struct container_list* temp_container = (struct container_list*)kmalloc(sizeof(struct container_list), GFP_KERNEL);
            temp_container->cid = kernel_cmd->cid;
            temp_container->head = temp_thread;
            temp_container->current = temp_container->head;
            temp_container->next = NULL;
            c = temp_container;
        }
    }
    return 0;
}

/**
 * switch to the next task in the next container
 * 
 * external functions needed:
 * mutex_lock(), mutex_unlock(), wake_up_process(), set_current_state(), schedule()
 */
int processor_container_switch(struct processor_container_cmd __user *user_cmd)
{
    // struct processor_container_cmd *kernel_cmd;
    // kernel_cmd = kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    // copy_from_user(kernel_cmd, user_cmd, sizeof(*user_cmd));
    // printk(KERN_INFO "Hello world %llu..\n", kernel_cmd->cid);
    return 0;
}

/**
 * control function that receive the command in user space and pass arguments to
 * corresponding functions.
 */
int processor_container_ioctl(struct file *filp, unsigned int cmd,
                              unsigned long arg)
{
    switch (cmd)
    {
    case PCONTAINER_IOCTL_CSWITCH:
        return processor_container_switch((void __user *)arg);
    case PCONTAINER_IOCTL_CREATE:
        return processor_container_create((void __user *)arg);
    case PCONTAINER_IOCTL_DELETE:
        return processor_container_delete((void __user *)arg);
    default:
        return -ENOTTY;
    }
}
