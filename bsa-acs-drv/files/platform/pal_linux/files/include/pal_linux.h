/*
 * BSA ACS Platform module.
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
 * Copyright (C) 2016-2018,2021 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#ifndef __PAL_LINUX_H__
#define __PAL_LINUX_H__

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/delay.h>

#include "val/include/pal_interface.h"

#define PAL_LINUX_SUCCESS  0x0
#define PAL_LINUX_ERR  0xEDCB1234  //some impropable value?
#define PAL_LINUX_SKIP 0x10000000

typedef enum {
    PER_FLAG_MSI_ENABLED = 0x2
}PER_FLAGS_e;

#define DMA_NOT_SUPPORTED  0x0
#define DMA_COHERENT       0x1
#define DMA_NOT_COHERENT   0x2
#define DMA_COHERENT_MASK  0xF

#define IOMMU_ATTACHED      0x10
#define IOMMU_ATTACHED_MASK 0xF0

#define PCI_EP      0x100
#define PCI_EP_MASK 0xF00

uint64_t pal_get_iort_ptr(void);

uint64_t pal_get_mcfg_ptr(void);

uint64_t pal_get_madt_ptr(void);

#define MIN_NUM_MSG 20
#define NUM_MSG_GROW(n) n*2
typedef struct __PAL_BSA_MSG__ {
    char string[92];
    unsigned long data;
}pal_msg_parms_t;

#define MSG_SIZE sizeof(pal_msg_parms_t)

extern char *g_msg_buf;
extern int tail_msg;
extern int num_msg;

extern uint32_t g_print_level;

int pal_smmu_check_dev_attach(struct device *dev);

#define ACS_PRINT_ERR   5      /* Only Errors. use this to de-clutter the terminal and focus only on specifics */
#define ACS_PRINT_WARN  4      /* Only warnings & errors. use this to de-clutter the terminal and focus only on specifics */
#define ACS_PRINT_TEST  3      /* Test description and result descriptions. THIS is DEFAULT */
#define ACS_PRINT_DEBUG 2      /* For Debug statements. contains register dumps etc */
#define ACS_PRINT_INFO  1      /* Print all statements. Do not use unless really needed */

#define bsa_print(verbosity, string, data) \
                                 if(verbosity >= g_print_level) {  \
                                     pal_print(string, data);  \
                                 }
#endif
