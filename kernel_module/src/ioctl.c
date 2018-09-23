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
    struct thread_list* cur;
    struct container_list* next;
};

struct thread_list
{
    struct task_struct* thread;
    struct thread_list* next;
};

extern struct mutex container_mutex;
extern struct container_list* start;

/**
 * Delete the task in the container.
 * 
 * external functions needed:
 * mutex_lock(), mutex_unlock(), wake_up_process(), 
 */
int processor_container_delete(struct processor_container_cmd __user *user_cmd)
{
    // mutex_lock(&container_mutex);
    // printk(KERN_INFO "final pid = %d cid = %llu..\n", start->head->thread->pid, start->cid);
    // printk(KERN_INFO "final pid = %d cid = %llu..\n", start->head->next->thread->pid, start->cid);
    // printk(KERN_INFO "final pid = %d cid = %llu..\n", start->head->next->next->thread->pid, start->cid);
    // printk(KERN_INFO "final pid = %d cid = %llu..\n", start->next->head->thread->pid, start->next->cid);
    // printk(KERN_INFO "final pid = %d cid = %llu..\n", start->next->head->next->thread->pid, start->next->cid);
    // mutex_unlock(&container_mutex);
    // struct processor_container_cmd *kernel_cmd;
    // kernel_cmd = kmalloc(sizeof(*user_cmd), GFP_KERNEL);
    // copy_from_user(kernel_cmd, user_cmd, sizeof(*user_cmd));
    // printk(KERN_INFO "Hello world %llu..\n", kernel_cmd->cid);

    // mutex_lock(&container_mutex);
    // struct container_list* temp_container = start;
    // struct thread_list* temp_thread;
    // while(temp_container != NULL)
    // {
    //     printk(KERN_INFO "\ncid = %llu..\n",temp_container->cid);
    //     temp_thread = temp_container->head;
    //     while(temp_thread != NULL)
    //     {
    //         printk(KERN_INFO "\npid = %d..\n",temp_thread->thread->pid);
    //         temp_thread = temp_thread->next;
    //     }
    //     temp_container = temp_container->next;
    // }
    // mutex_unlock(&container_mutex);

    // printk(KERN_INFO "fpid = %d..\n",current->pid);
    // mutex_lock(&container_mutex);
    // struct task_struct* ttt = start->head->next->thread;
    // wake_up_process(ttt);
    // mutex_unlock(&container_mutex);

    // printk(KERN_INFO "kpid = %d..\n",current->pid);
    struct container_list* temp_container = start;
    struct container_list* prev_container = NULL;
    mutex_lock(&container_mutex);
    while(temp_container != NULL)
    {
        if(current->pid == temp_container->cur->thread->pid)
        {
            if(temp_container->cur == temp_container->head && temp_container->head->next == NULL)
            {
                if(prev_container == NULL)
                {
                    start = start->next;
                }
                else
                {
                    prev_container->next = temp_container->next;
                }
                kfree(temp_container->head);
                kfree(temp_container);
            }
            else if(temp_container->cur == temp_container->head)
            {
                struct thread_list* temp_thread = temp_container->head;
                temp_container->head = temp_thread->next;
                temp_container->cur = temp_container->head;
                kfree(temp_thread);
                wake_up_process(temp_container->cur->thread);
            }
            else
            {
                struct thread_list* temp_thread = temp_container->head;
                while(temp_thread->next != temp_container->cur)
                {
                    temp_thread = temp_thread->next;
                }
                temp_thread->next = temp_container->cur->next;
                temp_thread = temp_container->cur;
                temp_container->cur = temp_thread->next;
                if(temp_container->cur == NULL)
                {
                    temp_container->cur = temp_container->head;
                }
                kfree(temp_thread);
                wake_up_process(temp_container->cur->thread);
            }
            break;
        }
        prev_container = temp_container;
        temp_container = temp_container->next;
    }
    mutex_unlock(&container_mutex);
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
    struct thread_list* temp_thread = (struct thread_list*)kmalloc(sizeof(struct thread_list), GFP_KERNEL);
    struct processor_container_cmd *kernel_cmd = kmalloc(sizeof(*user_cmd), GFP_KERNEL);

    temp_thread->thread = current;
    temp_thread->next = NULL;

    copy_from_user(kernel_cmd, user_cmd, sizeof(*user_cmd));

    mutex_lock(&container_mutex);
    // printk(KERN_INFO "pid = %d..\n",current->pid);
    if(start == NULL)
    {
        struct container_list* temp_container = (struct container_list*)kmalloc(sizeof(struct container_list), GFP_KERNEL);
        temp_container->cid = kernel_cmd->cid;
        temp_container->head = temp_thread;
        temp_container->cur = temp_container->head;
        temp_container->next = NULL;
        start = temp_container;
    }
    else
    {
        int flag = 0;
        struct container_list* c = start;
        while(c != NULL)
        {
            if(c->cid == kernel_cmd->cid)
            {
                struct thread_list* temp2 = c->head;
                flag = 1;
                while(temp2->next != NULL)
                {
                    temp2 = temp2->next;
                }
                temp2->next = temp_thread;
                // printk(KERN_INFO "lpid = %d..\n",current->pid);
                mutex_unlock(&container_mutex);
                set_current_state(TASK_INTERRUPTIBLE);
                schedule();
                mutex_lock(&container_mutex);
                break;
            }
            c = c->next;
        }
        if(flag == 0)
        {
            struct container_list* temp_container = (struct container_list*)kmalloc(sizeof(struct container_list), GFP_KERNEL);
            temp_container->cid = kernel_cmd->cid;
            temp_container->head = temp_thread;
            temp_container->cur = temp_container->head;
            temp_container->next = NULL;
            c = start;
            while(c->next != NULL)
            {
                c = c->next;
            }
            c->next = temp_container;
        }
    }
    mutex_unlock(&container_mutex);
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
    // printk(KERN_INFO "spid = %d..\n",current->pid);
    struct container_list* temp_container = start;
    mutex_lock(&container_mutex);
    while(temp_container != NULL)
    {
        if(current->pid == temp_container->cur->thread->pid && temp_container->head->next != NULL)
        {
            struct thread_list* temp_thread = temp_container->cur->next;
            if(temp_thread == NULL)
            {
                temp_thread = temp_container->head;
            }
            temp_container->cur = temp_thread;
            wake_up_process(temp_container->cur->thread);
            mutex_unlock(&container_mutex);
            set_current_state(TASK_INTERRUPTIBLE);
            schedule();
            mutex_lock(&container_mutex);
            break;
        }
        temp_container = temp_container->next;
    }
    mutex_unlock(&container_mutex);
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
