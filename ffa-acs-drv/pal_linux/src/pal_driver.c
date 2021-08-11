/*
 * PSA FFA ACS Platform module.
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
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <asm/io.h>

#include "pal_interfaces.h"
#include "pal_smmuv3_testengine.h"

#define MSG_SIZE 80

uint32_t pal_printf(const char *msg, uint64_t data1, uint64_t data2)
{
    char buf[MSG_SIZE];

    snprintf(buf, MSG_SIZE, msg, data1, data2);
    printk("%s", buf);

    return PAL_SUCCESS;
}

uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size)
{
    /* Empty api - not in use */
    (void)offset;
    (void)buffer;
    (void)size;
    return PAL_SUCCESS;
}

uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size)
{
    /* Empty api - not in use */
    (void)offset;
    (void)buffer;
    (void)size;
    return PAL_SUCCESS;
}

uint32_t pal_watchdog_enable(void)
{
    /* Empty api - not in use */
    return PAL_SUCCESS;
}

uint32_t pal_watchdog_disable(void)
{
    /* Empty api - not in use */
    return PAL_SUCCESS;
}

uint32_t pal_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure)
{
    return smmuv3_configure_testengine(stream_id, source, dest, size, secure);
}

MODULE_LICENSE("GPL");
