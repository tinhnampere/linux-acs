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
 * Copyright (C) 2016-2018 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include "include/pal_linux.h"
#include <linux/acpi.h>
#include <acpi/platform/aclinux.h>
#include <acpi/actypes.h>
#include <acpi/actbl.h>
#include <acpi/actbl1.h>

/**
  @brief  This API fills in the PE_INFO Table with information about the PEs in the
          system. This is achieved by parsing the ACPI - MADT table.

  @param  PeTable  - Address where the PE information needs to be filled.

  @return  None
**/
void
pal_pe_create_info_table(PE_INFO_TABLE *PeTable)
{
  unsigned int                       length=0, table_length=0;
  PE_INFO_ENTRY                      *ptr;
  struct acpi_table_madt             *madt;
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
          ptr->mpidr    = entry->arm_mpidr;
          ptr->pe_num   = PeTable->header.num_of_pe;
          ptr->pmu_gsiv = entry->performance_interrupt;
          sbsa_print(AVS_PRINT_DEBUG, "MPIDR %llx PE num %x \n", ptr->mpidr, ptr->pe_num);
          ptr++;
          PeTable->header.num_of_pe++;
      }
      length += entry->header.length;
      entry = (struct acpi_madt_generic_interrupt *) ((u8 *)entry + (entry->header.length));

  } while(length < table_length);
}

/**
  @brief  Install Exception Handler using UEFI CPU Architecture protocol's
          Register Interrupt Handler API

  @param  ExceptionType  - AARCH64 Exception type
  @param  esr            - Function pointer of the exception handler

  @return status of the API
**/
unsigned int
pal_pe_install_esr(unsigned int ExceptionType,  void (*esr)(unsigned long long, void *))
{

  return 0;
}

void
pal_pe_increment_elr(unsigned long long offset)
{
}

void
pal_pe_data_cache_ops_by_va(unsigned long long addr, unsigned type)
{
}

/**
  @brief  Make the SMC call using AARCH64 Assembly code

  @param  Argumets to pass to the EL3 firmware

  @return  None
**/
void
pal_pe_call_smc(ARM_SMC_ARGS *ArmSmcArgs)
{
}


/**
  @brief  Make a PSCI CPU_ON call using SMC instruction.
          Pass PAL Assembly code entry as the start vector for the PSCI ON call

  @param  Argumets to pass to the EL3 firmware

  @return  None
**/
void
pal_pe_execute_payload(ARM_SMC_ARGS *ArmSmcArgs)
{
}

void
pal_pe_update_elr(void *context, unsigned long long offset)
{
}

unsigned long long
pal_pe_get_far(void *context)
{
  return 0;
}

unsigned long long
pal_pe_get_esr(void *context)
{
  return 0;
}
