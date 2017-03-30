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

#include "include/pal_linux.h"

#include <linux/pci.h>
#include <linux/acpi.h>
#include <linux/pci-acpi.h>


uint64_t
pal_pcie_get_mcfg_ecam(void)
{
    struct acpi_table_mcfg      *mcfg;
    struct acpi_mcfg_allocation *cfg_table;

    mcfg = (struct acpi_table_mcfg *)pal_get_mcfg_ptr();

    if (!mcfg)
        return 0;

    cfg_table = (struct acpi_mcfg_allocation *) &mcfg[1];

    if (cfg_table)
        return cfg_table->address;
    else
        return 0;
}


/**
  @brief  Fill the PCIE Info table with the details of the PCIe sub-system
 **/
void
pal_pcie_create_info_table(PCIE_INFO_TABLE *PcieTable)
{
    unsigned int i = 0;

    PcieTable->block[i].ecam_base =  0x40000000; //pal_pcie_get_mcfg_ecam();
    PcieTable->block[i].start_bus_num = 0;
    PcieTable->block[i].segment_num = 0;
    PcieTable->num_entries = 1;

    return;
}

