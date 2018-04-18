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

#include <linux/acpi.h>
#include <acpi/actypes.h>
#include <acpi/actbl.h>
#include <acpi/actbl1.h>
#include <acpi/actbl3.h>
#include "pal_interface.h"
#include "pal_linux.h"

#define INVALID_INDEX 0xFFFFFFFF

typedef enum {
	TIMER_INFO_IS_PLATFORM_TIMER_SECURE,
	TIMER_INFO_SYS_INTID,
	TIMER_INFO_SYS_CNT_BASE_N
} sys_timer_info_t;

timer_info_table_t *g_sys_timertable;
/**
 * @breif  This API fills in the TIMER_INFO_TABLE with information about
 *   	   local and system timers in the system. This is achieved by parsing the ACPI - GTDT table.
 *
 * @param  timertable  Address where the Timer information needs to be filled.
 *
 * @return  None
 */
void pal_timer_create_info_table(timer_info_table_t *timertable)
{
	struct acpi_table_header *table;
	struct acpi_table_gtdt *gtdt;
	uint32_t length = 0;
	int32_t i;
	uint32_t num_of_entries;
	timer_info_gtblock_t *gt_entry = NULL;
	struct acpi_gtdt_timer_block *block_entry = NULL;
	struct acpi_gtdt_timer_entry *timer_entry = NULL;

	if (!timertable) {
		pal_print(ACS_LOG_ERR, "\n        Input timer table pointer is NULL");
		return;
	}

	if (ACPI_FAILURE(acpi_get_table(ACPI_SIG_GTDT, 0, &table))) {
		pal_print(ACS_LOG_ERR, "\n        not found GTDT");
		return;
	}

	if (!table) {
		pal_print(ACS_LOG_ERR, "\n        Failed to get GTDT table entry");
		return;
	}

	gt_entry = timertable->gt_info;
	timertable->header.num_platform_timer = 0;
	gtdt = container_of(table, struct acpi_table_gtdt, header);
	length = gtdt->platform_timer_offset;

	/* Fill in our internal table */
	timertable->header.s_el1_timer_flag = gtdt->secure_el1_flags;
	timertable->header.ns_el1_timer_flag = gtdt->non_secure_el1_flags;
	timertable->header.el2_timer_flag = gtdt->non_secure_el2_flags;
	timertable->header.s_el1_timer_gsiv = gtdt->secure_el1_interrupt;
	timertable->header.ns_el1_timer_gsiv = gtdt->non_secure_el1_interrupt;
	timertable->header.el2_timer_gsiv = gtdt->non_secure_el2_interrupt;
	timertable->header.virtual_timer_flag = gtdt->virtual_timer_flags;
	timertable->header.virtual_timer_gsiv = gtdt->virtual_timer_interrupt;

	block_entry = (struct acpi_gtdt_timer_block *)((uint8_t *)gtdt + length);
	length = sizeof(struct acpi_gtdt_timer_entry);
	num_of_entries = gtdt->platform_timer_count;

	while (num_of_entries) {
		if (block_entry->header.type == ACPI_GTDT_TYPE_TIMER_BLOCK) {
			pal_print(ACS_LOG_INFO, "\n        Found block entry");
			gt_entry->type = TIMER_TYPE_SYS_TIMER;
			gt_entry->block_cntl_base = block_entry->block_address;
			gt_entry->timer_count = block_entry->timer_count;
			pal_print(ACS_LOG_INFO, "\n        Block Control Base:%llx", gt_entry->block_cntl_base);
			timer_entry = (struct acpi_gtdt_timer_entry *)((uint8_t *)block_entry +
														block_entry->timer_offset);
			for (i = 0; i < gt_entry->timer_count; i++) {
				pal_print(ACS_LOG_INFO, "\n        Found timer entry");
				gt_entry->GtCntBase[i] = timer_entry->base_address;
				gt_entry->GtCntEl0Base[i] = timer_entry->el0_base_address;
				gt_entry->gsiv[i] = timer_entry->timer_interrupt;
				gt_entry->virt_gsiv[i] = timer_entry->virtual_timer_interrupt;
				gt_entry->flags[i] = timer_entry->timer_flags |
									 (timer_entry->virtual_timer_flags << 8) |
									 (timer_entry->common_flags << 16);
				pal_print(ACS_LOG_DEBUG, "\n        CNTBaseN:%llx sys counter:%d",
															gt_entry->GtCntBase[i], i);
				timer_entry++;
				timertable->header.num_platform_timer++;
			}
			gt_entry++;
		}
		block_entry = (struct acpi_gtdt_timer_block *)((uint8_t *)block_entry +
																block_entry->header.length);
		num_of_entries--;
	}
	g_sys_timertable = timertable;
}

static void pal_platform_timer_get_entry_index(uint32_t index, uint32_t *block,
															   uint32_t *block_index)
{
	if (index > g_sys_timertable->header.num_platform_timer){
		*block = INVALID_INDEX;
		return;
	}
	*block = 0;
	*block_index = index;
	while (index > g_sys_timertable->gt_info[*block].timer_count) {
		index = index - g_sys_timertable->gt_info[*block].timer_count;
		*block_index   = index;
		*block   = *block + 1;
	}
}

static uint64_t pal_timer_get_info(uint32_t type, uint32_t index)
{
	uint32_t block_num, block_index;

	if (!g_sys_timertable)
		return INVALID_INDEX;

	switch (type) {
		case TIMER_INFO_IS_PLATFORM_TIMER_SECURE:
			pal_platform_timer_get_entry_index (index, &block_num, &block_index);
			if (block_num != INVALID_INDEX)
				return ((g_sys_timertable->gt_info[block_num].flags[block_index] >> 16) & 1);
			break;
		case TIMER_INFO_SYS_INTID:
			pal_platform_timer_get_entry_index (index, &block_num, &block_index);
			if (block_num != INVALID_INDEX)
				return g_sys_timertable->gt_info[block_num].gsiv[block_index];
			break;
		case TIMER_INFO_SYS_CNT_BASE_N:
			pal_platform_timer_get_entry_index (index, &block_num, &block_index);
			if (block_num != INVALID_INDEX)
				return g_sys_timertable->gt_info[block_num].GtCntBase[block_index];
			break;
		default:
			break;
	}

	return INVALID_INDEX;
}

acs_status_t pal_configure_second_interrupt(uint32_t *index, uint64_t *int_id)
{
	uint64_t timer_num = g_sys_timertable->header.num_platform_timer;
	uint32_t ns_timer= 0;

	if (!timer_num) {
		pal_print(ACS_LOG_ERR, "\n        No System timers are defined");
		return ACS_ERROR;
	}

	while (timer_num && !ns_timer) {
		/* array index starts from 0, so subtract 1 from count */
		timer_num--;
		/* Skip secure timer */
		if (pal_timer_get_info(TIMER_INFO_IS_PLATFORM_TIMER_SECURE, timer_num))
			continue;

		ns_timer++;

		*int_id = pal_timer_get_info(TIMER_INFO_SYS_INTID, timer_num);
		*index = timer_num;
	}

	if (!ns_timer) {
		pal_print(ACS_LOG_ERR, "\n        No non-secure systimer implemented");
		return ACS_ERROR;
	}

	return ACS_SUCCESS;
}

/**
 * @breif This API fills in the WD_INFO_TABLE with information about Watchdogs
 *        in the system. This is achieved by parsing the ACPI - GTDT table.
 *
 * @param  wdtable  Address where the Timer information needs to be filled.
 *
 * @return  success or fail
 */
uint32_t pal_wd_create_info_table(wd_info_table_t *wdtable)
{
	struct acpi_table_header *table;
	struct acpi_gtdt_watchdog *wd_entry;
	wd_info_entry_t *wd_info = NULL;
	struct acpi_table_gtdt *gtdt;
	uint32_t length = 0;
	uint32_t num_of_entries;

	if (!wdtable) {
		pal_print(ACS_LOG_ERR, "\n        Input timer table pointer is NULL");
		return 1;
	}

	if (ACPI_FAILURE(acpi_get_table(ACPI_SIG_GTDT, 0, &table))) {
		pal_print(ACS_LOG_ERR, "\n        not found GTDT");
		return 1;
	}

	if (!table) {
		pal_print(ACS_LOG_ERR, "\n        Failed to get GTDT table entry");
		return 1;
	}

	wd_info = wdtable->wd_info;
	wdtable->header.num_wd = 0;
	gtdt = container_of(table, struct acpi_table_gtdt, header);
	length = gtdt->platform_timer_offset;
	num_of_entries = gtdt->platform_timer_count;
	wd_entry = (struct acpi_gtdt_watchdog *)((u8 *)gtdt + length);

	while (num_of_entries) {
		if (wd_entry->header.type == ACPI_GTDT_TYPE_WATCHDOG) {
			wd_info->wd_refresh_base = wd_entry->refresh_frame_address;
			wd_info->wd_ctrl_base	= wd_entry->control_frame_address;
			wd_info->wd_gsiv		 = wd_entry->timer_interrupt;
			wd_info->wd_flags		= wd_entry->timer_flags;
			wdtable->header.num_wd++;
			pal_print(ACS_LOG_INFO, "\n        Found Watchdog timer interrupt_id %d base %llx",
														   wd_info->wd_gsiv, wd_info->wd_ctrl_base);
		}
		wd_entry = (struct acpi_gtdt_watchdog*)((u8 *)wd_entry + (wd_entry->header.length));
		num_of_entries--;
	}
	return 0;
}

void pal_wd_set_ws0(uint64_t *vaddr, uint64_t base, uint32_t index, uint32_t timeout)
{
	if (!vaddr)
		return;

	if (timeout == 0) {
		pal_va_write(vaddr, WD_REG_BASE, WD_DISABLE);
		return;
	}

	pal_va_write(vaddr,  WD_REG_CTRL, timeout);
	pal_va_write(vaddr, WD_REG_BASE, WD_ENABLE);
}

void pal_generate_second_interrupt(uint32_t index, uint32_t timeout)
{
	uint64_t cnt_base_n;

	cnt_base_n = pal_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, index);
	pal_mmio_write(cnt_base_n + 0x28, timeout);
	pal_mmio_write(cnt_base_n + 0x2C, 1);
}

void pal_disable_second_interrupt(uint32_t index)
{
	uint64_t cnt_base_n;

	cnt_base_n = pal_timer_get_info(TIMER_INFO_SYS_CNT_BASE_N, index);
	pal_mmio_write(cnt_base_n + 0x2C, 0);
}
