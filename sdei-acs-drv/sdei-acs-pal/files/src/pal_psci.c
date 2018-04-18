/*
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
#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/uaccess.h>
#include <linux/psci.h>
#include <linux/acpi.h>

/*
 * The call to use to reach the firmware.
 */
int32_t pal_invoke_psci_fn(unsigned long function_id, unsigned long arg0,
										 unsigned long arg1, unsigned long arg2)
{
	struct arm_smccc_res res;

	arm_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	return res.a0;
}

