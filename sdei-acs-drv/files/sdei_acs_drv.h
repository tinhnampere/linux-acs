/*
 * SDEI Kernel module header file.
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

#ifndef __SDEI_ACS_DRV_H__
#define __SDEI_ACS_DRV_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MIN_NUM_MSG 20
#define SDEI_TEST_COMPLETE 0xFFFFFFFF
#define SDEI_TEST_CLEANUP    0xAAAAAAAA

typedef struct sdei_control {
    sdei_log_control log_control;
    sdei_test_control tst_control;
}sdei_control_t;

typedef struct __SDEI_MSG__ {
    char string[100];
    unsigned long data;
}test_msg_parms_t;
#endif
