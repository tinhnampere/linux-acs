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

#include "include/pal_linux.h"
#include "include/pal_dt.h"
#include <linux/of.h>
#include <linux/pci.h>

static u64 __init of_get_cpu_mpidr(struct device_node *dn)
{
    const __be32 *cell;
    u64 hwid;

    /*
     * A cpu node with missing "reg" property is
     * considered invalid to build a cpu_logical_map
     * entry.
     */
    cell = of_get_property(dn, "reg", NULL);
    if (!cell) {
        pr_err("%pOF: missing reg property\n", dn);
        return INVALID_HWID;
    }

    hwid = of_read_number(cell, of_n_addr_cells(dn));
    /*
     * Non affinity bits must be set to 0 in the DT
     */
    if (hwid & ~MPIDR_HWID_BITMASK) {
        pr_err("%pOF: invalid reg property\n", dn);
        return INVALID_HWID;
    }

    return hwid;
}

void
pal_pcie_create_info_table_dt(PCIE_INFO_TABLE *PcieTable)
{
  struct device_node *dn = NULL;
  const __be32 *reg, *bus_values;
  int32_t len;
  uint32_t i = 0;

  PcieTable->num_entries = 0;

  while ((dn = of_find_compatible_node(dn, NULL, "pci-host-ecam-generic"))) {
      reg = of_get_property(dn, "reg", &len);
      if (!reg || len < sizeof(*reg) * 4) {
      pr_err("%s: missing reg property\n", dn->full_name);
          return;
      }

      bus_values = of_get_property(dn, "bus-range", &len);
      if (!bus_values || len < sizeof(*bus_values) * 2) {
          pr_err("%s: Invalid/Missing bus range\n", dn->full_name);
          return;
      }

      PcieTable->block[i].ecam_base = of_read_number(reg, of_n_addr_cells(dn));
      PcieTable->block[i].start_bus_num =  be32_to_cpup(bus_values++);
      PcieTable->block[i].end_bus_num =  be32_to_cpup(bus_values);
      PcieTable->block[i].segment_num = 0; //TBD
      PcieTable->num_entries++;

      pr_debug("Ecam base: %llx\n", PcieTable->block[i].ecam_base);
      pr_debug("start bus num: %x\n",  PcieTable->block[i].start_bus_num);
      pr_debug("end bus num: %x\n",  PcieTable->block[i].end_bus_num);

      i++;

  }

  return;

}

void
pal_pe_create_info_table_dt(PE_INFO_TABLE *PeTable)
{

  PE_INFO_ENTRY                      *ptr;
  struct device_node *dn = NULL;
  u64 hwid;

  PeTable->header.num_of_pe = 0;
  ptr = PeTable->pe_info;
  //
  // Get from Device Tree
  //
  while ((dn = of_find_node_by_type(dn, "cpu"))) {
      hwid = of_get_cpu_mpidr(dn);

      if (hwid == INVALID_HWID) {
          pr_err("%s: Invalid HWID\n", dn->full_name);
          return;
      }

      ptr->mpidr    = hwid;
      ptr->pe_num   = PeTable->header.num_of_pe;
      ptr->pmu_gsiv = 0;  //TBD
      pr_debug("MPIDR %llx PE num %x \n", ptr->mpidr, ptr->pe_num);
      ptr++;
      PeTable->header.num_of_pe++;
  }

  return;
}

void
pal_iovirt_create_info_table_dt(IOVIRT_INFO_TABLE *iovirt_table)
{
  struct device_node *dn = NULL;
  const __be32 *reg;
  int32_t len, i = 0;

  while ((dn = of_find_compatible_node(dn, NULL, "arm,smmu-v1"))) {
      reg = of_get_property(dn, "reg", &len);
      if (!reg || len < sizeof(*reg) * 2) {
          pr_err("%s: missing compatible property\n", dn->full_name);
          return;
      }

      iovirt_table->blocks[i].type = IOVIRT_NODE_SMMU;
      iovirt_table->blocks[i].data.smmu.arch_major_rev = 1;
      iovirt_table->blocks[i].data.smmu.base = be32_to_cpup(reg);
      iovirt_table->num_smmus++;
      i++;
  }

  while ((dn = of_find_compatible_node(dn, NULL, "arm,smmu-v2"))){
      reg = of_get_property(dn, "reg", &len);
      if (!reg || len < sizeof(*reg) * 2) {
          pr_err("%s: missing compatible property\n", dn->full_name);
          return;
      }

      iovirt_table->blocks[i].type = IOVIRT_NODE_SMMU;
      iovirt_table->blocks[i].data.smmu.arch_major_rev = 2;
      iovirt_table->blocks[i].data.smmu.base = be32_to_cpup(reg);
      iovirt_table->num_smmus++;
      i++;
  }

    while ((dn = of_find_compatible_node(dn, NULL, "arm,smmu-v3"))) {
      reg = of_get_property(dn, "reg", &len);
      if (!reg || len < sizeof(*reg) * 2) {
          pr_err("%s: missing compatible property\n", dn->full_name);
          return;
      }

      iovirt_table->blocks[i].type = IOVIRT_NODE_SMMU_V3;
      iovirt_table->blocks[i].data.smmu.arch_major_rev = 3;
      iovirt_table->blocks[i].data.smmu.base = be32_to_cpup(reg);
      iovirt_table->num_smmus++;
      i++;
  }

  return;
}
