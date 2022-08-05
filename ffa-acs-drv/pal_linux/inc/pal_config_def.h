/*
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
 * Copyright (C) 2021 Arm Limited
 *
 */

#ifndef _PAL_CONFIG_H_
#define _PAL_CONFIG_H_

#ifndef __ASSEMBLY__
#include "pal_arch_helpers.h"
#endif

#define PLATFORM_PAGE_SIZE 0x1000

#define PAGE_SIZE_4K        0x1000
#define PAGE_SIZE_16K       (4 * 0x1000)
#define PAGE_SIZE_64K       (16 * 0x1000)

/* Non-secure UART assigned to VM1 - PL011_UART2_BASE */
#define PLATFORM_NS_UART_BASE    0x1c0b0000
#define PLATFORM_NS_UART_SIZE    0x10000

/* Secure UART assigned to SP1 - PL011_UART2_BASE */
#define PLATFORM_S_UART_BASE    0x1c0b0000
#define PLATFORM_S_UART_SIZE    0x10000

/* Non-volatile memory range assigned to SP3 */
#define PLATFORM_NVM_BASE    (0x80000000+0x2800000)
#define PLATFORM_NVM_SIZE    0x10000

/* Base address of watchdog assigned to SP3 */
#define PLATFORM_WDOG_BASE    0x1C0F0000 //(SP805)
#define PLATFORM_WDOG_SIZE    0x10000
#define PLATFORM_WDOG_LOAD_VALUE (0x3E7 * 10 * 1000) // 10sec
#define PLATFORM_WD_INTR_NUM 32

/* Base address of trusted watchdog (SP805) */
#define PLATFORM_SP805_TWDOG_BASE    0x2A490000
#define PLATFORM_TWDOG_SIZE          0x10000
#define PLATFORM_TWDOG_INTID         56

#define PLATFORM_NS_WD_BASE  0x2A440000
#define PLATFORM_NS_WD_SIZE  0x1000
#define PLATFORM_NS_WD_INTR  59

/*
 * Base address of read only memory region
 * TODO: Need to update read_only memory
 */
#define PLATFORM_MEM_READ_ONLY_BASE  0x00000000
#define PLATFORM_MEM_READ_ONLY_SIZE  0x1000

/* SMMU stream id */
#define PLATFORM_SMMU_STREAM_ID   1

/*
 * Retrieving of memory region by specifying address ranges.
 * this is about retrieving memory without handle value and
 * specifying the offset field in the endpoint memory access descriptor.
 * Supported values: 0(NOT_SUPPORTED), 1(SUPPORTED)
 */
#define PLATFORM_MEM_RETRIEVE_USING_ADDRESS_RANGES 0

/* INNER_SHAREBLE and OUTER_SHAREABLE flag support
 * Supported values: 0(NOT_SUPPORTED), 1(SUPPORTED)
 */
#define PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY 0
#define PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY 1
#define PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT 0

/*
 * Secure Partition manifest info.
 */

/* Test Partition IDs assigned by the system */
#define PLATFORM_SP1_ID             (1 | (1 << 15))
#define PLATFORM_SP2_ID             (2 | (1 << 15))
#define PLATFORM_SP3_ID             (3 | (1 << 15))

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
    #if (PLATFORM_PRIMARY_SCHEDULER_EL == 2)
        #define PLATFORM_VM1_ID              1
        #define PLATFORM_VM2_ID              2
        #define PLATFORM_VM3_ID              3
    #elif (PLATFORM_PRIMARY_SCHEDULER_EL == 1)
        // Assuming Primary scheduler ID   = 1
        #define PLATFORM_VM1_ID              2
        #define PLATFORM_VM2_ID              3
        #define PLATFORM_VM3_ID              4
    #endif
#endif

/*
 * CPU info
 */
#define PLATFORM_NO_OF_CPUS 8
#endif /* _PAL_CONFIG_H_ */
