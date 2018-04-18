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
 */

#include "pal_linux.h"
#include "pal_interface.h"
#include <linux/acpi.h>
#include <acpi/platform/aclinux.h>
#include <acpi/actypes.h>
#include <acpi/actbl.h>
#include <acpi/actbl1.h>

/**
 *  @brief  This API fills in the PE_INFO Table with information about the PEs in the
 *		  system. This is achieved by parsing the ACPI - MADT table.
 *
 *  @param  PeTable  - Address where the PE information needs to be filled.
 *
 *  @return  None
*/
void
pal_pe_create_info_table(pe_info_table_t *PeTable)
{
	unsigned int				length=0, table_length=0;
	pe_info_entry_t			 *ptr;
	struct acpi_table_madt	  *madt;
	struct acpi_madt_generic_interrupt *entry;


	madt = (struct acpi_table_madt *)pal_get_madt_ptr();

	if (!madt)
		return;

	table_length = madt->header.length;
	length = sizeof(struct acpi_table_madt);
	entry = (struct acpi_madt_generic_interrupt *) &madt[1];
	PeTable->header.num_of_pe = 0;
	ptr = PeTable->pe_info;

	do {
		if (entry->header.type == ACPI_MADT_TYPE_GENERIC_INTERRUPT) {
			ptr->mpidr	= entry->arm_mpidr;
			ptr->pe_num   = PeTable->header.num_of_pe;
			ptr->pmu_gsiv = entry->performance_interrupt;
			pal_print(ACS_LOG_DEBUG, "\n        MPIDR %llx PE num %x", ptr->mpidr, ptr->pe_num);
			ptr++;
			PeTable->header.num_of_pe++;
		}
		length += entry->header.length;
		entry = (struct acpi_madt_generic_interrupt *) ((u8 *)entry + (entry->header.length));

	} while(length < table_length);
}

int pal_pe_execute_on_all(int num_pe, void *func, uint64_t info)
{
	on_each_cpu(func, (void *)info, true);
	return 0;
}

void pal_pe_clean_up(void)
{
	return;
}

void
pal_pe_data_cache_invalidate(uint64_t Addr)
{
	return;
}

void
pal_pe_data_cache_clean_invalidate(uint64_t Addr)
{
	return;
}

void pal_pe_suspend(uint32_t power_state)
{
	return;
}

void pal_pe_poweroff(uint32_t pe_index)
{
	return;
}

void pal_pe_poweron(uint64_t pe_mpidr)
{
	return;
}
