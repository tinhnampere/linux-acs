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
 * Copyright (C) 2021 Arm Limited
 *
 */

#ifndef __PAL_DT_H__
#define __PAL_DT_H__

#define DT_IORT_NODE_SMMU_V1      1
#define DT_IORT_NODE_SMMU_V2      2
#define DT_IORT_NODE_SMMU_V3      3

void pal_pe_create_info_table_dt(PE_INFO_TABLE *PeTable);

void pal_pcie_create_info_table_dt(PCIE_INFO_TABLE *PcieTable);

void pal_iovirt_create_info_table_dt(IOVIRT_INFO_TABLE *iovirt_table);
#endif


