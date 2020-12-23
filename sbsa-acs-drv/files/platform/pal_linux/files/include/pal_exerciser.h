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
 * Copyright (C) 2016-2020 Arm Limited
 *
 */

#ifndef __PAL_EXERCISER_H__
#define __PAL_EXERCISER_H__

#include <linux/kernel.h>

#define MAX_ARRAY_SIZE 32
#define TEST_REG_COUNT 10
#define TEST_DDR_REGION_CNT 16

typedef struct {
    uint64_t buf[MAX_ARRAY_SIZE];
} EXERCISER_INFO_BLOCK;

typedef struct {
    uint32_t                num_exerciser_cards;
    EXERCISER_INFO_BLOCK    info[];  //Array of information blocks - per stimulus generation controller
} EXERCISER_INFO_TABLE;

typedef enum {
    EXERCISER_NUM_CARDS = 0x1
} EXERCISER_INFO_TYPE;

typedef enum {
    EDMA_NO_SUPPORT   = 0x0,
    EDMA_COHERENT     = 0x1,
    EDMA_NOT_COHERENT = 0x2,
    EDMA_FROM_DEVICE  = 0x3,
    EDMA_TO_DEVICE    = 0x4
} EXERCISER_DMA_ATTR;

typedef enum {
    SNOOP_ATTRIBUTES = 0x1,
    LEGACY_IRQ       = 0x2,
    MSIX_ATTRIBUTES  = 0x3,
    DMA_ATTRIBUTES   = 0x4,
    P2P_ATTRIBUTES   = 0x5,
    PASID_ATTRIBUTES = 0x6
} EXERCISER_PARAM_TYPE;

typedef enum {
    EXERCISER_RESET = 0x1,
    EXERCISER_ON    = 0x2,
    EXERCISER_OFF   = 0x3,
    EXERCISER_ERROR = 0x4
} EXERCISER_STATE;

typedef enum {
    START_DMA     = 0x1,
    GENERATE_MSI  = 0x2,
    GENERATE_L_INTR = 0x3,  //Legacy interrupt
    MEM_READ      = 0x4,
    MEM_WRITE     = 0x5,
    CLEAR_INTR    = 0x6,
    PASID_TLP_START = 0x7,
    PASID_TLP_STOP  = 0x8,
    TXN_NO_SNOOP_ENABLE  = 0x9,
    TXN_NO_SNOOP_DISABLE = 0xa,
    START_TXN_MONITOR    = 0xb,
    STOP_TXN_MONITOR     = 0xc
} EXERCISER_OPS;

typedef enum {
    ACCESS_TYPE_RD = 0x0,
    ACCESS_TYPE_RW = 0x1
} ECAM_REG_ATTRIBUTE;

struct ecam_reg_data {
    uint32_t offset;    //Offset into 4096 bytes ecam config reg space
    uint32_t attribute;
    uint32_t value;
};

struct exerciser_data_cfg_space {
    struct ecam_reg_data reg[TEST_REG_COUNT];
};

typedef enum {
    DEVICE_nGnRnE = 0x0,
    DEVICE_nGnRE  = 0x1,
    DEVICE_nGRE   = 0x2,
    DEVICE_GRE    = 0x3
} ARM_DEVICE_MEM;

typedef enum {
    NORMAL_NC = 0x4,
    NORMAL_WT = 0x5
} ARM_NORMAL_MEM;

typedef enum {
    MMIO_PREFETCHABLE = 0x0,
    MMIO_NON_PREFETCHABLE = 0x1
} BAR_MEM_TYPE;

struct exerciser_data_bar_space {
    void *base_addr;
    BAR_MEM_TYPE type;
};

typedef union exerciser_data {
    struct exerciser_data_cfg_space cfg_space;
    struct exerciser_data_bar_space bar_space;
} exerciser_data_t;

typedef enum {
    EXERCISER_DATA_CFG_SPACE = 0x1,
    EXERCISER_DATA_BAR0_SPACE = 0x2,
} EXERCISER_DATA_TYPE;


void pal_exerciser_create_info_table(EXERCISER_INFO_TABLE *exerciser_info_table);
uint32_t pal_exerciser_get_info(EXERCISER_INFO_TYPE type, uint32_t instance);
uint32_t pal_exerciser_set_param(EXERCISER_PARAM_TYPE type, uint64_t value1, uint64_t value2, uint32_t instance);
uint32_t pal_exerciser_get_param(EXERCISER_PARAM_TYPE type, uint64_t *value1, uint64_t *value2, uint32_t instance);
uint32_t pal_exerciser_set_state(EXERCISER_STATE state, uint64_t *value, uint32_t instance);
uint32_t pal_exerciser_get_state(EXERCISER_STATE *state, uint32_t bdf);
uint32_t pal_exerciser_ops(EXERCISER_OPS ops, uint64_t param, uint32_t instance);
uint32_t pal_exerciser_get_data(EXERCISER_DATA_TYPE type, exerciser_data_t *data, uint32_t instance);

#endif
