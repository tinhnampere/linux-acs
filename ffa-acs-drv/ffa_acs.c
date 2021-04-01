/*
 * FFA ACS Kernel module Main file.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2021 Arm Limited
 *
 */

#include "ffa_acs.h"

static int ffa_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    return 0;
}

static int ffa_proc_release(struct inode *sp_indoe, struct file *sp_file)
{
    return 0;
}

static ssize_t ffa_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
    return 0;
}

static ssize_t ffa_proc_write(struct file *sp_file, const char __user *buf,
                              size_t size, loff_t *offset)
{
    val_main();

    return size;
}

struct file_operations ffa_fops = {
    .open = ffa_proc_open,
    .read = ffa_proc_read,
    .write = ffa_proc_write,
    .release = ffa_proc_release
};

/* Create proc entry for communication between user and kernel levels */
static int __init init_ffa_acs(void)
{
    printk(KERN_INFO "\tInit FFA ACS driver \n");

    if (!proc_create("ffa_acs", 0, NULL, &ffa_fops)) {
        printk(KERN_ERR "ERROR! proc_create\n");
        remove_proc_entry("ffa_acs", NULL);
        return -1;
    }

    return 0;
}

/* Remove the proc entry while unloading the driver */
static void __exit exit_ffa_acs(void)
{
    remove_proc_entry("ffa_acs", NULL);
    printk(KERN_INFO "Exit FFA ACS driver \n");
}

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
module_init(init_ffa_acs);
module_exit(exit_ffa_acs);
