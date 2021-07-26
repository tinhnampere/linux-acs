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
 * Copyright (C) 2016-2019,2021 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/acpi_iort.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/bsa-iommu.h>
#include <linux/libata.h>

#include "include/pal_linux.h"

#define SMMU_V3_IDR1 0x4
#define SMMU_V3_IDR1_PASID_SHIFT 6
#define SMMU_V3_IDR1_PASID_MASK  0x1f

int pal_smmu_check_dev_attach(struct device *dev)
{
    if (!dev)
        return 0;

    return (iommu_get_domain_for_dev(dev) == NULL) ? 0 : 1;
}

void
pal_smmu_device_start_monitor_iova(void *port)
{
    if (!pal_smmu_check_dev_attach(((struct ata_port *)port)->dev)) {
        bsa_print(ACS_PRINT_WARN, "\n       This device is not behind an SMMU ", 0);
        return;
    }

    bsa_iommu_dev_start_monitor(((struct ata_port *)port)->dev);
}

void
pal_smmu_device_stop_monitor_iova(void *port)
{
    if (!pal_smmu_check_dev_attach(((struct ata_port *)port)->dev)) {
        bsa_print(ACS_PRINT_WARN, "\n       This device is not behind an SMMU ", 0);
        return;
    }

    bsa_iommu_dev_stop_monitor(((struct ata_port *)port)->dev);
}

/**
  @brief   This API checks if the input DMA address is part of the Device IOVA address table.
           1. Caller       -  Validation Abstraction Layer.
           2. Prerequisite -  pal_smmu_device_start_monitor_iova
  @param   port     - Device port whose domain IOVA table is checked
  @param   dma_addr - DMA address which is checked
  @return  status   - SUCCESS if the input address is part of the IOVA range.
**/
unsigned int
pal_smmu_check_device_iova(void *port, unsigned long long dma_addr)
{
    void *curr_node = NULL;
    unsigned int index = 0;
    unsigned long long base;
    unsigned long int  size;
    phys_addr_t phys;

    if (!pal_smmu_check_dev_attach(((struct ata_port *)port)->dev)) {
        bsa_print(ACS_PRINT_WARN, "\n       This device is not behind an SMMU ", 0);
        return PAL_LINUX_SKIP;
    }

    /* Check if this address was used in the last few transactions of the IOMMU layer */

    do {
        size = bsa_iommu_iova_get_addr(index, &base);
        if (size) {
            if ((dma_addr >= base) && (dma_addr < (base + size))) {
                return PAL_LINUX_SUCCESS;
            }

            index++;
        }

    } while(size);

    /* Did not find it above - Check the active IOVA table entries now */
    do {
        curr_node = bsa_iommu_dma_get_iova(((struct ata_port *)port)->dev, &base, &size, &phys, curr_node);
        if (curr_node) {
            pr_info("Device IOVA entry is %llx size = %lx phys = %llx \n", base, size, phys);
            if ((dma_addr >= base) && (dma_addr < (base + size))) {
                return PAL_LINUX_SUCCESS;
            }
        }
    } while(curr_node);

    return PAL_LINUX_ERR;
}

uint32_t
pal_smmu_max_pasids(uint64_t smmu_base)
{
  uint32_t reg = pal_mmio_read(smmu_base + SMMU_V3_IDR1);
  uint32_t pasid_bits = reg >> SMMU_V3_IDR1_PASID_SHIFT & SMMU_V3_IDR1_PASID_MASK;
  if(pasid_bits == 0)
     return 0;
  return (1 << pasid_bits);
}

uint32_t
pal_smmu_create_pasid_entry(uint64_t smmu_base, uint32_t pasid)
{
  return 0;
}

uint32_t
pal_smmu_disable(uint64_t smmu_base)
{
  return 0;
}

uint64_t
pal_smmu_pa2iova(uint64_t smmu_base, uint64_t pa)
{
  return 0;
}
