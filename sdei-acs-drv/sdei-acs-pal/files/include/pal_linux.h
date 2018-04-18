/*
 * SDEI ACS Platform module.
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
 *
 */

#ifndef __PAL_LINUX_H__
#define __PAL_LINUX_H__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include "pal_interface.h"

uint64_t pal_get_iort_ptr(void);

uint64_t pal_get_mcfg_ptr(void);

uint64_t pal_get_madt_ptr(void);

#define NUM_MSG_GROW(n) n*2
extern sdei_log_control g_log_control;
extern char *g_msg_buf;
extern int g_tail_msg;

typedef struct __PAL_SDEI_MSG__ {
	char string[100];
	unsigned long data;
} pal_msg_parms_t;

#endif /* __PAL_LINUX_H__ */
