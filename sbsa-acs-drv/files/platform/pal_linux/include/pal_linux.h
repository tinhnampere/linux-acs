/*
 * SBSA ACS Platform module.
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

#ifndef __PAL_LINUX_H__
#define __PAL_LINUX_H__

#include <linux/kernel.h>
#include <linux/slab.h>
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
typedef struct __PAL_SBSA_MSG__ {
    char string[92];
    unsigned long data;
}pal_msg_parms_t;

#define MSG_SIZE sizeof(pal_msg_parms_t)

extern char *g_msg_buf;
extern int tail_msg;
extern int num_msg;

extern uint32_t g_print_level;

#define AVS_PRINT_ERR   5      /* Only Errors. use this to de-clutter the terminal and focus only on specifics */
#define AVS_PRINT_WARN  4      /* Only warnings & errors. use this to de-clutter the terminal and focus only on specifics */
#define AVS_PRINT_TEST  3      /* Test description and result descriptions. THIS is DEFAULT */
#define AVS_PRINT_DEBUG 2      /* For Debug statements. contains register dumps etc */
#define AVS_PRINT_INFO  1      /* Print all statements. Do not use unless really needed */

#define sbsa_print(verbosity, string, ...)  \
                                 if(verbosity >= g_print_level) {\
                                     char buf[sizeof(pal_msg_parms_t)], *tmp; \
                                     if(tail_msg >= num_msg) { \
                                       tmp = kmalloc(NUM_MSG_GROW(num_msg) * sizeof(pal_msg_parms_t), GFP_KERNEL); \
                                       if(tmp) { \
                                         memcpy(tmp, g_msg_buf, num_msg * sizeof(pal_msg_parms_t)); \
                                         num_msg = NUM_MSG_GROW(num_msg); \
                                         kfree(g_msg_buf); \
                                         g_msg_buf = tmp; \
                                       } else \
                                         tail_msg = tail_msg % num_msg; \
                                     } \
                                     sprintf(buf, string, ##__VA_ARGS__); \
                                     memcpy(g_msg_buf+(tail_msg*sizeof(pal_msg_parms_t)), buf, sizeof(buf));\
                                     tail_msg = (tail_msg+1); \
                                 }

#endif
