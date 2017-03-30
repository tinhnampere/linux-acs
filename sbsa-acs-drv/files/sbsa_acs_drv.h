/*
 * SBSA ACS Kernel module header file.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm-generic/uaccess.h>

#include <linux/slab.h>

#define SBSA_CREATE_INFO_TABLES 0x1000
#define SBSA_EXECUTE_TEST       0x2000
#define SBSA_FREE_INFO_TABLES   0x9000


/* STATUS MESSAGES */
#define DRV_STATUS_AVAILABLE     0x10000000
#define DRV_STATUS_PENDING       0x40000000


typedef
struct __TEST_PARAMS__
{
    unsigned int  api_num;
    unsigned int  num_pe;
    unsigned int  level;
    unsigned long arg0;
    unsigned long arg1;
    unsigned long arg2;
}test_params_t;
