/*
 * Software Delegated Exception Interface (SDEI) driver code.
 *
 * Copyright (C) 2018 Arm Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/spinlock.h>
#include <linux/arm-smccc.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/psci.h>
#include <asm/smp_plat.h>
#include <linux/acpi.h>

#include "pal_sdei_interface.h"
#include "pal_interface.h"
#include "pal_linux.h"

/*
 * The call to use to reach the firmware.
 */
static asmlinkage void (*sdei_firmware_call)(unsigned long function_id,
			  unsigned long arg0, unsigned long arg1,
			  unsigned long arg2, unsigned long arg3,
			  unsigned long arg4, struct arm_smccc_res *res);

static DEFINE_SPINLOCK(sdei_events_lock);

int pal_sdei_to_linux_errno(unsigned long sdei_err)
{
	switch (sdei_err) {
	case SDEI_NOT_SUPPORTED:
		return SDEI_STATUS_NOT_SUPPORTED;
	case SDEI_INVALID_PARAMETERS:
		return SDEI_STATUS_INVALID;
	case SDEI_DENIED:
		return SDEI_STATUS_DENIED;
	case SDEI_PENDING:
		return SDEI_STATUS_PENDING;
	case SDEI_OUT_OF_RESOURCE:
		return SDEI_STATUS_OUT_OF_RESOURCE;
	}

	/* Not an error value ... */
	return sdei_err;
}

/*
 * If x0 is any of these values, then the call failed, use sdei_to_linux_errno()
 * to translate.
 */
static int sdei_is_err(struct arm_smccc_res *res)
{
	switch (res->a0) {
	case SDEI_NOT_SUPPORTED:
	case SDEI_INVALID_PARAMETERS:
	case SDEI_DENIED:
	case SDEI_PENDING:
	case SDEI_OUT_OF_RESOURCE:
		return true;
	}

	return false;
}

int pal_invoke_sdei_fn(unsigned long function_id, unsigned long arg0,
			  unsigned long arg1, unsigned long arg2,
			  unsigned long arg3, unsigned long arg4,
			  u64 *result)
{
	int err = 0;
	struct arm_smccc_res res;

	if (sdei_firmware_call) {
		sdei_firmware_call(function_id, arg0, arg1, arg2, arg3, arg4,
				   &res);
		if (sdei_is_err(&res))
			err = pal_sdei_to_linux_errno(res.a0);
	} else {
		/*
		 * !sdei_firmware_call means we failed to probe or called
		 * sdei_mark_interface_broken(). -EIO is not an error returned
		 * by sdei_to_linux_errno() and is used to suppress messages
		 * from this driver.
		 */
		err = SDEI_STATUS_NOT_SUPPORTED;
		res.a0 = SDEI_NOT_SUPPORTED;
	}

	if (result)
		*result = res.a0;

	return err;
}

void *pal_intf_alloc(int size)
{
	return kzalloc(size, GFP_KERNEL);
}

void pal_intf_free(void *handle)
{
	kfree(handle);
}

unsigned int pal_smp_pe_id(void)
{
	return smp_processor_id();
}

void pal_intf_lock(void)
{
	spin_lock(&sdei_events_lock);
}

void pal_intf_unlock(void)
{
	spin_unlock(&sdei_events_lock);
}

static void sdei_smccc_smc(unsigned long function_id,
			   unsigned long arg0, unsigned long arg1,
			   unsigned long arg2, unsigned long arg3,
			   unsigned long arg4, struct arm_smccc_res *res)
{
	arm_smccc_smc(function_id, arg0, arg1, arg2, arg3, arg4, 0, 0, res);
}

void pal_interface_broken(void)
{
	sdei_firmware_call = NULL;
}

int pal_conduit_get(void)
{
	sdei_firmware_call = &sdei_smccc_smc;
	return CONDUIT_SMC;
}

/*
 * Read ACPI Hardware Error Source table and intialize the event info
 * table with event numbers read from Generic Hardware Error Source
 * structures in the table.
 */
int32_t pal_sdei_create_event_info_table(event_info_table_t *EventTable)
{
	event_info_t *event;
	int32_t i;
	acpi_status status;
	struct acpi_hest_generic *ghes;
	struct acpi_table_hest *hest_table;

	EventTable->num_events = 0;
	EventTable->hest_found = 0;
	EventTable->num_ghes_notify = 0;
	if (acpi_disabled)
		return 1;

	status = acpi_get_table(ACPI_SIG_HEST, 0, (void *)&hest_table);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		const char *msg = acpi_format_exception(status);
		pal_print(ACS_LOG_ERR, "\n        Failed to get ACPI::HEST TABLE %s", msg);
		return 1;
	}

	if (!hest_table)
		return 1;

	EventTable->hest_found = 1;
	event = (event_info_t*) EventTable->info;

	/* Generic Hardware Error Source */
	ghes = (struct acpi_hest_generic *)(hest_table + 1);
	for (i=0; i < hest_table->error_source_count; i++) {
		if (ghes->notify.type == ACPI_HEST_NOTIFY_SDEI) {
			event->number = ghes->notify.vector;
			pal_print(ACS_LOG_DEBUG, "\n        Found SDEI error event %d", event->number);
			EventTable->num_ghes_notify++;
			if (event->number)
				EventTable->num_events++;
			event++;
		}
		if (ghes->header.type == ACPI_HEST_TYPE_GENERIC_ERROR_V2)
			ghes = (struct acpi_hest_generic *)((u8 *)ghes + sizeof(struct acpi_hest_generic_v2));
		else
			ghes++;
	}

	return 0;
}

int pal_acpi_present(void)
{
	acpi_status status;
	struct acpi_table_header *sdei_table_header;

	if (acpi_disabled)
		return false;

	status = acpi_get_table(ACPI_SIG_SDEI, 0, &sdei_table_header);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		const char *msg = acpi_format_exception(status);

		pal_print(ACS_LOG_ERR, "\n        Failed to get ACPI:SDEI table, %s", msg);
	}
	if (ACPI_FAILURE(status))
		return false;

	return true;
}
