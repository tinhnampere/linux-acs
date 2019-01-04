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
#include <linux/irq.h>
#include <linux/version.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>

static int8_t *dev = "SDEI";

/**
 *  @brief  Populate information about the GIC sub-system at the input address.
 *		  In a ACPI framework, this information is part of the MADT table.
 *
 *  @param  gic_table  Address of the memory region where this information is to be filled in
 *
 *  @return None
 */
void pal_gic_create_info_table(gic_info_table_t *gic_table)
{
	unsigned int							length=0, table_length=0;
	struct acpi_table_madt					*madt;
	struct acpi_madt_generic_interrupt		*entry;
	struct acpi_madt_generic_distributor	*dis_entry;
	struct acpi_madt_generic_redistributor  *redis_entry;
	struct acpi_madt_generic_translator		 *its_entry;
	gic_info_entry_t						*gic_entry = NULL;

	madt = (struct acpi_table_madt *)pal_get_madt_ptr();
	if (!madt)  {
		pal_print(ACS_LOG_ERR, "\n        Madt not found");
		return;
	}

	entry = (struct acpi_madt_generic_interrupt *) &madt[1];
	dis_entry = (struct acpi_madt_generic_distributor *) &madt[1];
	redis_entry = (struct acpi_madt_generic_redistributor *) &madt[1];
	its_entry = (struct acpi_madt_generic_translator *) &madt[1];
	table_length = madt->header.length;

	gic_entry = gic_table->gic_info;
	gic_table->header.gic_version = 0;
	gic_table->header.num_gicrd = 0;
	gic_table->header.num_gicd = 0;
	gic_table->header.num_its = 0;

	length = sizeof(struct acpi_table_madt);
	do {
		if (entry->header.type == ACPI_MADT_TYPE_GENERIC_INTERRUPT) {
			if (entry->base_address != 0) {
				gic_entry->type = ENTRY_TYPE_CPUIF;
				gic_entry->base = entry->base_address;
				gic_entry++;
			}
		}
			length += entry->header.length;
			entry = (struct acpi_madt_generic_interrupt *) ((u8 *)entry + (entry->header.length));
	} while(length < table_length);

	length = sizeof(struct acpi_table_madt);
	do {
		if (dis_entry->header.type == ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR) {
			if (dis_entry->base_address != 0) {
				gic_entry->type = ENTRY_TYPE_GICD;
				gic_entry->base = dis_entry->base_address;
				gic_table->header.gic_version = dis_entry->version;
				pal_print(ACS_LOG_INFO, "\n        GIC Distributor base %llx Version %d",
                                                 gic_entry->base, gic_table->header.gic_version);
				gic_table->header.num_gicd++;
				gic_entry++;
			}
		}
			length += dis_entry->header.length;
			dis_entry = (struct acpi_madt_generic_distributor *) ((u8 *)dis_entry + \
																(dis_entry->header.length));
	} while(length < table_length);

	length = sizeof(struct acpi_table_madt);
	do {
		if (redis_entry->header.type == ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR) {
			if (redis_entry->base_address != 0) {
				gic_entry->type = ENTRY_TYPE_GICRD;
				gic_entry->base = redis_entry->base_address;
				pal_print(ACS_LOG_INFO, "\n        GIC RD base %llx", gic_entry->base);
				gic_table->header.num_gicrd++;
				gic_entry++;
			}
		}
			length += redis_entry->header.length;
			redis_entry = (struct acpi_madt_generic_redistributor *) ((u8 *)redis_entry + \
																	(redis_entry->header.length));
	} while(length < table_length);

	length = sizeof(struct acpi_table_madt);
	do {
		if (its_entry->header.type == ACPI_MADT_TYPE_GENERIC_TRANSLATOR) {
			if (its_entry->base_address != 0) {
				gic_entry->type = ENTRY_TYPE_GICITS;
				gic_entry->base = its_entry->base_address;
				pal_print(ACS_LOG_INFO, "\n        GIC ITS base %llx", gic_entry->base);
				gic_table->header.num_its++;
				gic_entry++;
			}
		}
			length += its_entry->header.length;
			its_entry = (struct acpi_madt_generic_translator *) ((u8 *)its_entry + \
																(its_entry->header.length));
	} while(length < table_length);

	gic_entry->type = 0xFF;  //Indicate end of data
}

/**
 *  @brief  Enable the interrupt in the GIC Distributor and GIC CPU Interface and hook
 *		  the interrupt service routine for the IRQ to the UEFI Framework
 *
 *  @param  int_id  Interrupt ID which needs to be enabled and service routine installed for
 *  @param  isr	 Function pointer of the Interrupt service routine
 *
 *  @return Status of the operation
 */
uint32_t pal_gic_install_isr(uint32_t int_id, void *isr)
{
	uint64_t flags = 0;
	int32_t ret = 0;
	int32_t virq;
	struct irq_domain *domain = NULL;
	struct irq_fwspec *fwspec;

	domain = acs_get_irq_domain();
	if (!domain) {
		pal_print(ACS_LOG_ERR, "\n        Domain is null");
		return 1;
	}

	fwspec = kmalloc(sizeof(struct irq_fwspec), GFP_KERNEL);
	if (!fwspec) {
		pal_print(ACS_LOG_ERR, "\n        Kmalloc failed");
		return 1;
	}

	fwspec->param_count = 2;
	fwspec->param[0] = int_id;
	/* Interrupt type LEVEL 0 EDGE RISING 1 */
	fwspec->param[1] = 0;
	fwspec->param[2] = 0;
	fwspec->fwnode = domain->fwnode;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	fwspec->fwnode->type = FWNODE_IRQCHIP;
#endif

	if (irq_domain_is_hierarchy(domain)) {
		virq = irq_domain_alloc_irqs(domain, 1, NUMA_NO_NODE, fwspec);
		if (virq <= 0) {
			ret = 1;
			goto error;
		}
	} else {
		/* Create mapping */
		virq = irq_create_mapping(domain, int_id);
		if (!virq) {
			ret = 1;
			goto error;
		}
	}

	ret = request_irq(virq, (irq_handler_t)isr, flags, "SDEI", dev);
	if (ret != 0) {
		pal_print(ACS_LOG_ERR, "\n        ERROR: Cannot request IRQ %x err %d", int_id, ret);
		ret = 1;
		goto error;
	}

error:
	kfree(fwspec);
	return ret;
}

uint32_t pal_gic_free_interrupt(uint32_t int_id)
{
	uint32_t virq;
	struct irq_domain *domain = NULL;

	domain = acs_get_irq_domain();
	virq = irq_create_mapping(domain, int_id);
	free_irq(virq, dev);
	irq_dispose_mapping(virq);

	return 0;
}

void pal_gic_end_of_interrupt(uint32_t int_id)
{
	return;
}
