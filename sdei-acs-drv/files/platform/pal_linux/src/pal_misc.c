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

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <asm/io.h>
#include "pal_linux.h"
#include "pal_interface.h"

pe_shared_mem_t *g_pe_shared_mem;
extern int g_num_msg;

/**
 *  @brief This API provides a single point of abstraction to read from all
 *		   Memory Mapped IO address
 *
 *  @param  addr 64-bit address
 *
 *  @return 32-bit data read from the input address
 */
uint32_t pal_mmio_read(uint64_t addr)
{
	uint32_t data;
	uint64_t *p;

	if (addr & 0x3) {
		pal_print(ACS_LOG_WARN, "\n        Error-Input address is not aligned.");
		/* make sure addr is aligned to 4 bytes */
		addr = addr & ~(0x3);
	}
	p = ioremap(addr, 16);
	data = ioread32(p);
	iounmap(p);

	return data;
}

/**
 * @brief This API Provides a single point of abstraction to write to all
 *		  Memory Mapped IO address
 *
 * @param  addr  64-bit address
 * @param  data  32-bit data to write to address
 *
 * @return None
 */
void pal_mmio_write(uint64_t addr, uint32_t data)
{
	uint64_t *p;
	p = ioremap(addr, 16);
	iowrite32(data, p);

	iounmap(p);
}

uint64_t *pal_pa_to_va(uint64_t addr)
{
	uint64_t *va;

	va = ioremap(addr, 0x40);

	return va;
}

void pal_va_write(uint64_t *addr, uint32_t offset, uint32_t data)
{
	*(uint32_t *)((uint8_t *)addr + offset) = data;
}

void pal_va_free(uint64_t *addr)
{
	iounmap(addr);
}

void pal_print(uint32_t verbosity, char *string, ...)
{
	if (verbosity <= g_log_control.print_level) {
		char buf[sizeof(pal_msg_parms_t)], *tmp=NULL;
		va_list args;

		va_start(args, string);
		if (g_tail_msg >= g_num_msg) {
			tmp = kmalloc(NUM_MSG_GROW(g_num_msg) * sizeof(pal_msg_parms_t), GFP_KERNEL);
			if (tmp) {
				memcpy(tmp, g_msg_buf, g_num_msg * sizeof(pal_msg_parms_t));
				g_num_msg = NUM_MSG_GROW(g_num_msg);
				kfree(g_msg_buf);
				g_msg_buf = tmp;
			} else
				g_tail_msg = g_tail_msg % g_num_msg;
		}
		vsprintf(buf, string, args);
		if (g_log_control.print_level == ACS_LOG_KERNEL)
			printk("%s", buf);
		va_end(args);
		memcpy(g_msg_buf+ (g_tail_msg * sizeof(pal_msg_parms_t)), buf, sizeof(buf));
		g_tail_msg = (g_tail_msg + 1);
	}
}

/**
 * @brief Allocate memory which is to be used to share data across PEs
 *
 * @param  num_pe	     Number of PEs in the system
 * @param  sizeofentry   Size of memory region allocated to each PE
 *
 * @return None
 */
acs_status_t pal_pe_alloc_shared_mem(uint32_t num_pe, size_t sizeofentry)
{
	g_pe_shared_mem = NULL;

	g_pe_shared_mem = kzalloc((num_pe * sizeofentry), GFP_KERNEL);
	if (!g_pe_shared_mem) {
		pal_print(ACS_LOG_ERR, "\n        Shared mem allocation failed");
		return ACS_ERROR;
	}

	pal_print(ACS_LOG_INFO, "\n        Shared memory %p", g_pe_shared_mem);
	return ACS_SUCCESS;
}

void pal_pe_free_shared_mem()
{
	kfree(g_pe_shared_mem);
}

void pal_free_mem(uint64_t *ptr)
{
	kfree(ptr);
}

void pal_print_raw(char *string, uint64_t data)
{
	return;
}
