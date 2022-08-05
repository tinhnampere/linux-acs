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
#include <acpi/platform/aclinux.h>
#include <acpi/actypes.h>
#include <acpi/actbl.h>
#include <acpi/actbl1.h>
#include <linux/irq.h>
#include <linux/version.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>
#include "pal_interfaces.h"
#include "pal_irq.h"

static int8_t *dev = "FFA";

void pal_irq_setup(void)
{
    return;
}

int pal_irq_handler_dispatcher(void)
{
    return 0;
}

void pal_irq_enable(unsigned int irq_num, uint8_t irq_priority)
{
    return;
}

void pal_irq_disable(unsigned int irq_num)
{
    return;
}

int pal_irq_register_handler(unsigned int irq_num, handler_irq_t irq_handler)
{
    uint64_t flags = 0;
    int32_t ret = 0;
    int32_t virq;
    struct irq_domain *domain = NULL;
    struct irq_fwspec *fwspec;

    domain = acs_get_irq_domain();
    if (!domain) {
        pal_printf("Domain is null\n", 0, 0);
        return PAL_ERROR;
    }

    fwspec = kmalloc(sizeof(struct irq_fwspec), GFP_KERNEL);
    if (!fwspec) {
        pal_printf("Fwspec allocation failed\n", 0, 0);
        return PAL_ERROR;
    }

    fwspec->param_count = 3;
    fwspec->param[0] = 0;
    /* Interrupt type LEVEL 0 EDGE RISING 1 */
    fwspec->param[1] = irq_num-32;
    fwspec->param[2] = 4;
    fwspec->fwnode = domain->fwnode;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
    fwspec->fwnode->type = FWNODE_IRQCHIP;
#endif

     if (irq_domain_is_hierarchy(domain)) {
         virq = __irq_domain_alloc_irqs(domain, -1, 1, NUMA_NO_NODE, fwspec, false, NULL);
         if (virq <= 0) {
             ret = PAL_ERROR;
             goto error;
         }
     } else {
         /* Create mapping */
         virq = irq_create_mapping(domain, irq_num);
         if (!virq) {
             ret = PAL_ERROR;
             goto error;
         }
     }

     ret = request_irq(virq, (irq_handler_t)irq_handler, flags, "FFA", dev);
     if (ret) {
         pal_printf("ERROR: Cannot request IRQ %x err %d\n", irq_num, ret);
         ret = PAL_ERROR;
         goto error;
     }

error:
    kfree(fwspec);
    return ret;
}

int pal_irq_unregister_handler(unsigned int irq_num)
{
    uint32_t virq;
    struct irq_domain *domain = NULL;

    domain = acs_get_irq_domain();
    virq = irq_create_mapping(domain, irq_num);
    free_irq(virq, dev);
    irq_dispose_mapping(virq);

    return PAL_SUCCESS;
}

void pal_send_sgi(unsigned int sgi_id, unsigned int core_pos)
{
    return;
}
