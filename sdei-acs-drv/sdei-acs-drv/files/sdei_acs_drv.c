/*
 * SDEI Kernel Interface Module
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
 * Copyright (C) 2018 Arm Limited
 *
 * Author: Mahesh Bireddy <mahesh.reddybireddy@arm.com>
 *
 */

#include "val_interface.h"
#include "sdei_acs_drv.h"

sdei_log_control g_log_control;

char *g_msg_buf;
int g_tail_msg;
int g_num_msg = MIN_NUM_MSG;
static unsigned int g_test_completion_flag = 0;

int create_pe_infotable(void)
{
	int ret = 0;
	uint64_t *pe_infotable;

	pe_infotable = kzalloc(PE_INFO_TABLE_SZ, GFP_KERNEL);
	if (!pe_infotable)
	{
		val_print(ACS_LOG_ERR, "\n        pe_infotable Allocation failed");
		return 1;
	}

	ret = val_pe_create_info_table(pe_infotable);

	return ret;
}

int create_gic_infotable(void)
{
	int ret = 0;
	uint64_t *gic_infotable;

	gic_infotable = kzalloc(GIC_INFO_TABLE_SZ, GFP_KERNEL);
	if (!gic_infotable)
	{
		val_print(ACS_LOG_ERR, "\n        gic_infotable Allocation failed");
		return 1;
	}

	ret = val_gic_create_info_table(gic_infotable);

	return ret;
}

int create_event_infotable(void)
{
	int ret = 0;
	uint64_t *event_infotable;

	event_infotable = kzalloc(EVENT_INFO_TABLE_SZ, GFP_KERNEL);
	if (!event_infotable)
	{
		val_print(ACS_LOG_ERR, "\n        event_infotable Allocation failed");
		return 1;
	}

	ret = val_sdei_create_event_info_table(event_infotable);

	return ret;
}

int create_watchdog_infotable(void)
{
	int ret = 0;
	uint64_t *wdinfotable;

	wdinfotable = kzalloc(WD_INFO_TABLE_SZ, GFP_KERNEL);
	if (!wdinfotable)
	{
		val_print(ACS_LOG_ERR, "\n        WdInfoTable Allocation failed");
		return 1;
	}

	ret = val_wd_create_info_table(wdinfotable);

	return ret;
}

int create_timer_infotable(void)
{
	uint64_t *timer_infotable;

	timer_infotable = kzalloc(TIMER_INFO_TABLE_SZ, GFP_KERNEL);
	if (!timer_infotable)
	{
		val_print(ACS_LOG_ERR, "\n        timer infotable Allocation failed");
		return 1;
	}

	val_timer_create_info_table(timer_infotable);

	return 0;
}

static int sdei_init(void)
{
	int ret = 0;

	g_msg_buf = (char *)kmalloc(g_num_msg * sizeof(test_msg_parms_t), GFP_KERNEL);

	ret = create_gic_infotable();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        gic_infotable failed");
		return ret;
	}

	ret = create_pe_infotable();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        pe_infotable failed");
		return ret;
	}

	ret = create_event_infotable();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        SDEI Event InfoTable failed");
		return ret;
	}

	ret = create_watchdog_infotable();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        SDEI Watchdog InfoTable failed");
		return ret;
	}

	ret = create_timer_infotable();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        Timer infotable failed");
		return ret;
	}

	ret = val_sdei_initialization();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        SDEI initialization failed");
		return ret;
	}

	return ret;
}

static int sdei_proc_open(struct inode *sp_inode, struct file *sp_file)
{
	return 0;
}

static int sdei_proc_release(struct inode *sp_indoe, struct file *sp_file)
{
	return 0;
}

static ssize_t sdei_proc_read(struct file *sp_file,char __user *buf, size_t size, loff_t *offset)
{
	int var;

	var = copy_to_user(buf, &g_test_completion_flag, sizeof(g_test_completion_flag));
	return sizeof(g_test_completion_flag);
}

static ssize_t sdei_proc_write(struct file *sp_file,const char __user *buf,
															size_t size, loff_t *offset)
{
	int var;
	int ret = 0;
	sdei_test_control control;
	sdei_control_t sdei_control;

	val_test_init(&control);
	var = copy_from_user(&sdei_control, buf, size);
	g_log_control = sdei_control.log_control;
	control = sdei_control.tst_control;

	ret = sdei_init();
	if (ret) {
		val_print(ACS_LOG_ERR, "\n        SDEI Initialization failed");
		g_test_completion_flag = SDEI_TEST_COMPLETE;
		return size;
	}
	val_shared_mem_alloc();
	val_test_execute(&control);

    /* Always print test results */
	val_print(ACS_LOG_ERR, "\nTests passed  : %d", control.tests_passed);
	val_print(ACS_LOG_ERR, "\nTests failed  : %d", control.tests_failed);
	val_print(ACS_LOG_ERR, "\nTests skipped : %d", control.tests_skipped);
	val_print(ACS_LOG_ERR, "\nTests aborted : %d", control.tests_aborted);

	val_shared_mem_free();
	g_test_completion_flag = SDEI_TEST_COMPLETE;
	return size;
}

static ssize_t sdei_msg_proc_write(struct file *sp_file,const char __user *buf,
														size_t size, loff_t *offset)
{
	unsigned int flag;
	int ret;

	ret = copy_from_user(&flag, buf, sizeof(flag));
	if (flag == SDEI_TEST_CLEANUP) {
		val_pe_free_info_table();
		val_gic_free_info_table();
		val_event_free_info_table();
		val_watchdog_free_info_table();
		val_timer_free_info_table();
		kfree(g_msg_buf);
	}
	return sizeof(flag);
}

static ssize_t sdei_msg_proc_read(struct file *sp_file,char __user *buf,
														size_t size, loff_t *offset)
{
	int len;
	int ret;

	len = sizeof(test_msg_parms_t);
	if (*offset == (g_tail_msg *len))
		len = 0;

	 ret = simple_read_from_buffer(buf, len, offset, g_msg_buf,
											g_num_msg * sizeof(test_msg_parms_t));
	 if (ret != 0)
		 memset(g_msg_buf+(*offset)-len, 0, sizeof(test_msg_parms_t));

	return ret;
}

struct file_operations sdei_msg_fops = {
	.open = sdei_proc_open,
	.read = sdei_msg_proc_read,
	.write = sdei_msg_proc_write,
	.release = sdei_proc_release
};

struct file_operations fops = {
	.open = sdei_proc_open,
	.read = sdei_proc_read,
	.write = sdei_proc_write,
	.release = sdei_proc_release
};

static int __init init_sdeiproc (void)
{
	if (!proc_create("sdei",0666,NULL,&fops)) {
		printk("ERROR! proc_create\n");
		remove_proc_entry("sdei",NULL);
		return -1;
	}

	if (!proc_create("sdei_msg",0666,NULL,&sdei_msg_fops)) {
		printk("ERROR! proc_create SBSA Msg \n");
		remove_proc_entry("sdei_msg",NULL);
		return -1;
	}

	return 0;
}

static void __exit exit_sdeiproc(void)
{
	remove_proc_entry("sdei",NULL);
	remove_proc_entry("sdei_msg",NULL);
}

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
module_init(init_sdeiproc);
module_exit(exit_sdeiproc);
