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
 * Copyright (C) 2016-2020 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include "sbsa_acs_drv.h"

#include "val/include/val_interface.h"
#include "val/include/sbsa_avs_common.h"
#include "val/include/sbsa_avs_val.h"
#include "val/include/sbsa_avs_pcie.h"
#include "val/include/sbsa_avs_exerciser.h"

test_params_t params;
test_msg_parms_t msg_params;

static int len = 0;

unsigned int  g_sbsa_level = 4;
unsigned int  g_print_level = 3;
unsigned int  g_skip_test_num[3];
unsigned int  g_sbsa_tests_total;
unsigned int  g_sbsa_tests_pass;
unsigned int  g_sbsa_tests_fail;
uint64_t g_exception_ret_addr;
uint64_t g_stack_pointer;
uint64_t g_ret_addr;
unsigned int g_print_mmio;
unsigned int g_curr_module;
unsigned int g_enable_module;


uint64_t  *g_pe_info_ptr;
uint64_t  *g_pcie_info_ptr;
uint64_t  *g_per_info_ptr;
uint64_t  *g_dma_info_ptr;
uint64_t  *g_iovirt_info_ptr;

char *g_msg_buf;
int tail_msg;
extern int num_msg;

int
val_glue_execute_command(void)
{
    g_print_level = params.arg1;
    if (params.api_num == SBSA_CREATE_INFO_TABLES)
    {
        g_sbsa_tests_total = 0;
        g_sbsa_tests_pass = 0;
        g_sbsa_tests_fail = 0;
        tail_msg = 0;

        g_msg_buf = (char*) kmalloc(num_msg * sizeof(test_msg_parms_t), GFP_KERNEL);

        g_pe_info_ptr = kmalloc(PE_INFO_TBL_SZ, GFP_KERNEL);
        val_pe_create_info_table(g_pe_info_ptr);

        g_pcie_info_ptr = kmalloc(PCIE_INFO_TBL_SZ, GFP_KERNEL);
        val_pcie_create_info_table(g_pcie_info_ptr);

        g_per_info_ptr = kmalloc(PERIPHERAL_INFO_TBL_SZ, GFP_KERNEL);
        val_peripheral_create_info_table(g_per_info_ptr);

        g_dma_info_ptr = kmalloc(DMA_INFO_TBL_SZ, GFP_KERNEL);
        val_dma_create_info_table(g_dma_info_ptr);

        g_iovirt_info_ptr = kmalloc(IOVIRT_INFO_TBL_SZ, GFP_KERNEL);
        val_iovirt_create_info_table(g_iovirt_info_ptr);

        val_allocate_shared_mem();

        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = 0;

    }

    if (params.api_num == SBSA_FREE_INFO_TABLES)
    {
        kfree(g_pe_info_ptr);
        kfree(g_pcie_info_ptr);
        kfree(g_per_info_ptr);
        kfree(g_dma_info_ptr);
        kfree(g_iovirt_info_ptr);
        kfree(g_msg_buf);

    }

    if (params.api_num == SBSA_EXERCISER_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        //val_exerciser_execute_tests(params.level);
        val_print(AVS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        val_print(AVS_PRINT_TEST, "\n      Total Tests Run = %2d, ", g_sbsa_tests_total);
        val_print(AVS_PRINT_TEST, "Tests Passed = %2d, ", g_sbsa_tests_pass);
        val_print(AVS_PRINT_TEST, "Tests Failed = %2d ", g_sbsa_tests_fail);
        val_print(AVS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if (params.api_num == SBSA_PCIE_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        val_pcie_execute_tests(AVS_PCIE_RCiEP_DISABLE, params.level, params.num_pe);
        val_print(AVS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        val_print(AVS_PRINT_TEST, "\n      Total Tests Run = %2d, ", g_sbsa_tests_total);
        val_print(AVS_PRINT_TEST, "Tests Passed = %2d, ", g_sbsa_tests_pass);
        val_print(AVS_PRINT_TEST, "Tests Failed = %2d ", g_sbsa_tests_fail);
        val_print(AVS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if(params.api_num == SBSA_UPDATE_SKIP_LIST){
        g_skip_test_num[0] = params.arg0;
        g_skip_test_num[1] = params.arg1;
        g_skip_test_num[2] = params.arg2;
    }

    return 0;
}

static
int sbsa_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    return 0;
}

static
int sbsa_proc_release(struct inode *sp_indoe, struct file *sp_file)
{
    return 0;
}

static
ssize_t sbsa_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{

    int var;
    var = copy_to_user(buf,&params,len);
    return len;
}

static
ssize_t sbsa_proc_write(struct file *sp_file,const char __user *buf, size_t size, loff_t *offset)
{
    int var;
    len = size;
    var = copy_from_user(&params,buf,len);
    val_glue_execute_command();
    return len;
}

static
ssize_t sbsa_msg_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{
    int length;
    len = sizeof(test_msg_parms_t);
    if(*offset == (tail_msg*len)) len=0;

    length = simple_read_from_buffer(buf, len, offset, g_msg_buf, num_msg * sizeof(test_msg_parms_t));

    if(length != 0)
      memset(g_msg_buf+(*offset)-len, 0, sizeof(test_msg_parms_t));

    return length;
}

struct file_operations sbsa_msg_fops = {
    .open = sbsa_proc_open,
    .read = sbsa_msg_proc_read,
    .release = sbsa_proc_release
};

struct file_operations fops = {
    .open = sbsa_proc_open,
    .read = sbsa_proc_read,
    .write = sbsa_proc_write,
    .release = sbsa_proc_release
};

static int __init init_sbsaproc (void)
{
    printk("init SBSA Driver \n");
    if (!proc_create("sbsa",0666,NULL,&fops)) {
        printk("ERROR! proc_create\n");
        remove_proc_entry("sbsa",NULL);
        return -1;
    }

    if (!proc_create("sbsa_msg",0666,NULL,&sbsa_msg_fops)) {
        printk("ERROR! proc_create SBSA Msg \n");
        remove_proc_entry("sbsa_msg",NULL);
        return -1;
    }

    return 0;
}

static
void __exit exit_sbsaproc(void)
{
    remove_proc_entry("sbsa",NULL);
    remove_proc_entry("sbsa_msg",NULL);
    printk("exit SBSA Driver \n");
}

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
module_init(init_sbsaproc);
module_exit(exit_sbsaproc);
