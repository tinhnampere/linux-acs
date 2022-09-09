/*
 * BSA ACS Kernel module Main file.
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
 * Copyright (C) 2016-2022 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/version.h>
#include "bsa_acs_drv.h"

#include "val/include/val_interface.h"
#include "val/include/bsa_acs_common.h"
#include "val/include/bsa_acs_val.h"
#include "val/include/bsa_acs_pcie.h"
#include "val/include/bsa_acs_exerciser.h"

test_params_t params;
test_msg_parms_t msg_params;

static int len = 0;

unsigned int  g_print_level = 3;
unsigned int  g_sw_view[3]; //Operating System, Hypervisor, Platform Security
unsigned int  g_skip_test_num[3];
unsigned int  g_bsa_tests_total;
unsigned int  g_bsa_tests_pass;
unsigned int  g_bsa_tests_fail;
uint64_t g_exception_ret_addr;
uint64_t g_stack_pointer;
uint64_t g_ret_addr;

unsigned int g_single_test = SINGLE_TEST_SENTINEL;
unsigned int g_single_module = SINGLE_MODULE_SENTINEL;

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
    if (params.api_num == BSA_CREATE_INFO_TABLES)
    {
        g_bsa_tests_total = 0;
        g_bsa_tests_pass = 0;
        g_bsa_tests_fail = 0;
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

    if (params.api_num == BSA_FREE_INFO_TABLES)
    {
        kfree(g_pe_info_ptr);
        kfree(g_pcie_info_ptr);
        kfree(g_per_info_ptr);
        kfree(g_dma_info_ptr);
        kfree(g_iovirt_info_ptr);
        kfree(g_msg_buf);

    }

    if (params.api_num == BSA_EXERCISER_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        //val_exerciser_execute_tests(params.level);
        val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        val_print(ACS_PRINT_TEST, "\n      Total Tests Run = %2d, ", g_bsa_tests_total);
        val_print(ACS_PRINT_TEST, "Tests Passed = %2d, ", g_bsa_tests_pass);
        val_print(ACS_PRINT_TEST, "Tests Failed = %2d ", g_bsa_tests_fail);
        val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------------\n", 0);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if (params.api_num == BSA_PCIE_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        val_pcie_execute_tests(params.num_pe, g_sw_view);

        val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------------", 0);
        val_print(ACS_PRINT_TEST, "\n      Total Tests Run = %2d, ", g_bsa_tests_total);
        val_print(ACS_PRINT_TEST, "Tests Passed = %2d, ", g_bsa_tests_pass);
        val_print(ACS_PRINT_TEST, "Tests Failed = %2d ", g_bsa_tests_fail);
        val_print(ACS_PRINT_TEST, "\n     ------------------------------------------------------------\n", 0);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if (params.api_num == BSA_PER_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        val_peripheral_execute_tests(params.num_pe, g_sw_view);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if (params.api_num == BSA_MEM_EXECUTE_TEST)
    {
        params.arg0 = DRV_STATUS_PENDING;
        val_memory_execute_tests(params.num_pe, g_sw_view);
        params.arg0 = DRV_STATUS_AVAILABLE;
        params.arg1 = val_get_status(0);
    }

    if(params.api_num == BSA_UPDATE_SKIP_LIST){
        g_skip_test_num[0] = params.arg0;
        g_skip_test_num[1] = params.arg1;
        g_skip_test_num[2] = params.arg2;
    }

    if(params.api_num == BSA_UPDATE_SW_VIEW){
        g_sw_view[0] = params.arg0;
        g_sw_view[1] = params.arg1;
        g_sw_view[2] = params.arg2;
    }

    return 0;
}

static
int bsa_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    return 0;
}

static
int bsa_proc_release(struct inode *sp_indoe, struct file *sp_file)
{
    return 0;
}

static
ssize_t bsa_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{

    int var;
    var = copy_to_user(buf,&params,len);
    return len;
}

static
ssize_t bsa_proc_write(struct file *sp_file,const char __user *buf, size_t size, loff_t *offset)
{
    int var;
    len = size;
    var = copy_from_user(&params,buf,len);
    val_glue_execute_command();
    return len;
}

static
ssize_t bsa_msg_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{
    int length;
    len = sizeof(test_msg_parms_t);
    if(*offset == (tail_msg*len)) len=0;

    length = simple_read_from_buffer(buf, len, offset, g_msg_buf, num_msg * sizeof(test_msg_parms_t));

    if(length != 0)
      memset(g_msg_buf+(*offset)-len, 0, sizeof(test_msg_parms_t));

    return length;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,6,0)
const struct proc_ops bsa_msg_fops = {
    .proc_open = bsa_proc_open,
    .proc_read = bsa_msg_proc_read,
    .proc_release = bsa_proc_release
};

static const struct proc_ops fops = {
    .proc_open = bsa_proc_open,
    .proc_read = bsa_proc_read,
    .proc_write = bsa_proc_write,
    .proc_release = bsa_proc_release
};
#else
struct file_operations bsa_msg_fops = {
    .open = bsa_proc_open,
    .read = bsa_msg_proc_read,
    .release = bsa_proc_release
};

struct file_operations fops = {
    .open = bsa_proc_open,
    .read = bsa_proc_read,
    .write = bsa_proc_write,
    .release = bsa_proc_release
};
#endif

static int __init init_bsaproc (void)
{
    printk("init BSA Driver \n");
    if (!proc_create("bsa",0666,NULL,&fops)) {
        printk("ERROR! proc_create\n");
        remove_proc_entry("bsa",NULL);
        return -1;
    }

    if (!proc_create("bsa_msg",0666,NULL,&bsa_msg_fops)) {
        printk("ERROR! proc_create BSA Msg \n");
        remove_proc_entry("bsa_msg",NULL);
        return -1;
    }

    return 0;
}

static
void __exit exit_bsaproc(void)
{
    remove_proc_entry("bsa",NULL);
    remove_proc_entry("bsa_msg",NULL);
    printk("exit BSA Driver \n");
}

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
module_init(init_bsaproc);
module_exit(exit_bsaproc);
