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
#include <linux/pci-acpi.h>
#include <linux/acpi_iort.h>
#include <acpi/actbl2.h>

uint64_t
pal_get_madt_ptr(void)
{
    struct acpi_table_header *madt_table;
    acpi_status status;

    status = acpi_get_table(ACPI_SIG_MADT, 0, &madt_table);

    return (uint64_t)madt_table;
}

uint64_t
pal_get_mcfg_ptr(void)
{
    struct acpi_table_header *mcfg_table;
    acpi_status status;

    status = acpi_get_table(ACPI_SIG_MCFG, 0, &mcfg_table);

    return (uint64_t)mcfg_table;

}

uint64_t
pal_get_iort_ptr(void)
{
    struct acpi_table_header *iort_table;
    acpi_status status;

    status = acpi_get_table(ACPI_SIG_IORT, 0, &iort_table);

    return (uint64_t)iort_table;
}
