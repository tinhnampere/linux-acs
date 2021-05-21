/*
 * BSA ACS Platform module.
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
 * Copyright (C) 2016-2021 Arm Limited
 *
 */

#include "include/pal_linux.h"
#include <linux/irq.h>
#include <linux/version.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>

unsigned int pal_gic_install_isr(unsigned int int_id, void (*isr)(void))
{
    unsigned long long flags = 0;
    unsigned int ret = 0;
    unsigned int virq;
    struct irq_domain *domain = NULL;
    struct irq_fwspec *fwspec;

    domain =  acs_get_irq_domain();
    if (!domain) {
        bsa_print(ACS_PRINT_ERR, "\n       Domain is null", 0);
        return 1;
    }

    fwspec = kmalloc(sizeof(struct irq_fwspec), GFP_KERNEL);
    if (!fwspec) {
        bsa_print(ACS_PRINT_ERR, "\n       Kmalloc failed", 0);
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

    ret = request_irq(virq, (irq_handler_t)isr, flags, "BSA", NULL);
    if (ret != 0) {
        bsa_print(ACS_PRINT_ERR, "\n       IRQ registration failure %x", int_id);
        bsa_print(ACS_PRINT_ERR, " \n      err %d", ret);
        ret = 1;
        goto error;
    }

error:
    kfree(fwspec);
    return ret;
}

void pal_gic_end_of_interrupt(unsigned int int_id)
{
  return;
}

uint32_t pal_gic_request_irq(unsigned int irq_num, unsigned int mapped_irq_num, void *isr)
{
    return request_irq(mapped_irq_num, (irq_handler_t)isr, 0, NULL, NULL);
}

void pal_gic_free_irq(unsigned int irq_num, unsigned int mapped_irq_num)
{
    free_irq(mapped_irq_num, NULL);
}

/* Placeholder for MSI related implementation*/
uint32_t pal_gic_request_msi(unsigned int its_id, unsigned int device_id,
                             unsigned int int_id, unsigned int msi_index,
                             unsigned int *msi_addr, unsigned int *msi_data)
{
  return 0;
}

/* Placeholder for MSI related implementation*/
void pal_gic_free_msi(unsigned int its_id, unsigned int device_id,
                      unsigned int int_id, unsigned int msi_index)
{
  return;
}

/* Placeholder for ITS Configuration */
uint32_t pal_gic_its_configure(void)
{
  return 0;
}

/* Placeholder for returning max LPI supported */
uint32_t pal_gic_get_max_lpi_id(void)
{
  return 0;
}
