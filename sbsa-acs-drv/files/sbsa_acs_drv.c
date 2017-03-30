/*
 * SBSA ACS Kernel module Main file.
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
 * Copyright (C) 2017 ARM Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include "sbsa_acs_drv.h"

#include "val/include/val_interface.h"
#include "val/include/sbsa_avs_common.h"
#include "val/include/sbsa_avs_val.h"
#include "val/include/sbsa_avs_pcie.h"

test_params_t params;

static int len = 0;

unsigned int  g_sbsa_level = 3;
unsigned int  g_print_level = 3;
unsigned int  g_skip_test_num[3];
unsigned int  g_sbsa_tests_total;
unsigned int  g_sbsa_tests_pass;
unsigned int  g_sbsa_tests_fail;


uint64_t  *g_pe_info_ptr;
uint64_t  *g_pcie_info_ptr;
uint64_t  *g_per_info_ptr;
uint64_t  *g_dma_info_ptr;
uint64_t  *g_iovirt_info_ptr;

int
val_glue_execute_command(void)
{

    if (params.api_num == SBSA_CREATE_INFO_TABLES)
    {

        g_pe_info_ptr = kmalloc(8192, GFP_KERNEL);
        val_pe_create_info_table(g_pe_info_ptr);

        g_pcie_info_ptr = kmalloc(1024, GFP_KERNEL);
        val_pcie_create_info_table(g_pcie_info_ptr);

        g_per_info_ptr = kmalloc(1024, GFP_KERNEL);
        val_peripheral_create_info_table(g_per_info_ptr);

        g_dma_info_ptr = kmalloc(1024, GFP_KERNEL);
        val_dma_create_info_table(g_dma_info_ptr);

        g_iovirt_info_ptr = kmalloc(4096, GFP_KERNEL);
        val_iovirt_create_info_table(g_iovirt_info_ptr);

        val_allocate_shared_mem();

        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = 0;

    }

    if (params.api_num == SBSA_FREE_INFO_TABLES)
    {
        kfree(g_pe_info_ptr);
        kfree(g_iovirt_info_ptr);

    }

    if (params.api_num == SBSA_EXECUTE_TEST)
    {

        switch (params.arg0)
        {
            case 51:
                p001_entry(params.num_pe);
                break;
            case 52:
                p002_entry(params.num_pe);
                break;
            case 53:
                p003_entry(params.num_pe);
                break;
            case 54:
                p004_entry(params.num_pe);
                break;
            case 55:
                p005_entry(params.num_pe);
                break;
            case 56:
                p006_entry(params.num_pe);
                break;
            case 57:
                p007_entry(params.num_pe);
                break;
           default:
                printk("Test number %ld not implemented \n", params.arg0);
                params.arg0 = DRV_STATUS_AVAILABLE;
                params.arg1 = RESULT_SKIP(3, params.arg0, 01);
        }
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }


  return 0;
}



static
int sbsa_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    //printk("proc called open\n");
    return 0;
}

static
int sbsa_proc_release(struct inode *sp_indoe, struct file *sp_file)
{
    //printk("proc called release\n");
    return 0;
}

static
ssize_t sbsa_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{

    //printk("proc called read %lx\n",size);
    copy_to_user(buf,&params,len);
    return len;
}

static
ssize_t sbsa_proc_write(struct file *sp_file,const char __user *buf, size_t size, loff_t *offset)
{

    //printk("proc called write %lx\n",size);
    len = size;
    copy_from_user(&params,buf,len);
    printk("Test parameters are %x %x \n", params.api_num, params.level);
    val_glue_execute_command();
    return len;
}

struct file_operations fops = {
    .open = sbsa_proc_open,
    .read = sbsa_proc_read,
    .write = sbsa_proc_write,
    .release = sbsa_proc_release
};

static int __init init_sbsaproc (void)
{
    printk("init SBSA Driver \n");
    if (! proc_create("sbsa",0666,NULL,&fops)) {
        printk("ERROR! proc_create\n");
        remove_proc_entry("sbsa",NULL);
        return -1;
    }
    return 0;
}

static
void __exit exit_sbsaproc(void)
{
    remove_proc_entry("sbsa",NULL);
    printk("exit SBSA Driver \n");
}

MODULE_LICENSE("GPL");
module_init(init_sbsaproc);
module_exit(exit_sbsaproc);
