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

/**
  @brief  This API fills in the PE_INFO Table with information about the PEs in the
          system. This is achieved by parsing the ACPI - MADT table.

  @param  PeTable  - Address where the PE information needs to be filled.

  @return  None
**/
void
pal_pe_create_info_table(PE_INFO_TABLE *PeTable)
{
  //EFI_ACPI_6_1_GIC_STRUCTURE    *Entry;
  PE_INFO_ENTRY                 *Ptr = PeTable->pe_info;


  PeTable->header.num_of_pe = 0;

  //Fill in the cpu num and the mpidr in pe info table
  Ptr->mpidr    = 0;
  Ptr->pe_num   = PeTable->header.num_of_pe;
  Ptr->pmu_gsiv = 27;
  //Print(L"FOUND an entry %x %x \n", Ptr->mpidr, Ptr->pe_num);
 // pal_pe_data_cache_ops_by_va((UINT64)Ptr, CLEAN_AND_INVALIDATE);
  Ptr++;
  PeTable->header.num_of_pe++;
  
}

/**
  @brief  Install Exception Handler using UEFI CPU Architecture protocol's
          Register Interrupt Handler API

  @param  ExceptionType  - AARCH64 Exception type
  @param  esr            - Function pointer of the exception handler

  @return status of the API
**/
unsigned int
pal_pe_install_esr(unsigned int ExceptionType,  void (*esr)(void))
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
pal_pe_update_elr(unsigned long long offset)
{ 
}


unsigned int
BigEndianCheck(unsigned long *ptr)
{

    return 0;
}
