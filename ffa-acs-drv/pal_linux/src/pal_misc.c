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
 * Author: Jaykumar P Patel <jaykumar.pitambarbhaipatel@arm.com>
 *
 */

#include <linux/slab.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include "pal_interfaces.h"

/* Note - This is unused for linux target */
uint32_t pal_get_endpoint_device_map(void **region_list, size_t *no_of_mem_regions)
{
    *region_list = NULL;
    *no_of_mem_regions = 0;

    return PAL_SUCCESS;
}

uint32_t pal_terminate_simulation(void)
{
   return PAL_SUCCESS;
}

void *pal_mem_virt_to_phys(void *va)
{
  return (void *)virt_to_phys(va);
}

void *pal_memory_alloc(uint64_t size)
{
    uint32_t order = (size/PAGE_SIZE_4K)/2;
    void *ptr;

    ptr = (void *)__get_free_pages(GFP_KERNEL, order);
    if (!ptr)
    {
        pal_printf("\tpal_memory_alloc failed \n", 0, 0);
        return NULL;
    }

    return ptr;
}

uint32_t pal_memory_free(void *address, uint64_t size)
{
    uint32_t order = (size/PAGE_SIZE_4K)/2;

    free_pages((uint64_t)address, order);

    return PAL_SUCCESS;
}

int pal_memcmp(void *src, void *dest, size_t len)
{
  return memcmp(src, dest, len);
}

void *pal_memset(void *dst, int val, size_t count)
{
  memset(dst, val, count);
  return dst;
}

void *pal_memcpy(void *dst, const void *src, size_t len)
{
  return memcpy(dst, src, len);
}
