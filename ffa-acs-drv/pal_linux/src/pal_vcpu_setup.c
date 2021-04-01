/*
 * FF-A ACS Platform module.
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
 * Author: Mahesh Bireddy <mahesh.reddybireddy@arm.com>
 *
 */

#include <linux/acpi.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include "pal_interfaces.h"

uint32_t pal_get_no_of_cpus(void)
{
    return num_online_cpus();
}

uint32_t pal_get_cpuid(uint64_t mpdir)
{
    return read_cpuid_id();
}

uint64_t pal_get_mpid(uint32_t cpuid)
{
    return read_cpuid_mpidr();
}

uint32_t pal_power_on_cpu(uint64_t mpidr)
{
    return PAL_SUCCESS;
}

uint32_t pal_power_off_cpu(void)
{
    return PAL_SUCCESS;
}

void pal_secondary_cpu_boot_entry(void)
{
    return;
}
