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

#endif
