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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "include/pal_linux.h"
#include "include/sbsa_pcie_enum.h"
//#include "include/platform_override.h"

#define USB_CLASSCODE   0x0C0330
#define SATA_CLASSCODE  0x010601
#define BAR0            0
#define BAR1            1
#define BAR5            5

void
pal_peripheral_create_dma_table(void);

void
pal_peripheral_create_info_table(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{

  struct pci_dev *pdev = NULL;
  PERIPHERAL_INFO_BLOCK *per_info;

  per_info = peripheralInfoTable->info;

  peripheralInfoTable->header.num_usb = 0;
  peripheralInfoTable->header.num_sata = 0;
  peripheralInfoTable->header.num_uart = 0;

  /* check for any XHCI - USB Controllers */
  do {

       pdev = pal_pci_get_dev(USB_CLASSCODE, pdev);
       if (pdev != 0) {
          per_info->type  = PERIPHERAL_TYPE_USB;
          per_info->base0 = pal_pcie_get_base(pdev, BAR0);
          per_info->bdf   = pal_pcie_get_bdf(pdev);
          per_info->flags = pci_dev_msi_enabled(pdev)? PER_FLAG_MSI_ENABLED : 0;
          per_info->irq   = pdev->irq;
          printk("Found a USB controller %4x \n", per_info->bdf);
          peripheralInfoTable->header.num_usb++;
          per_info++;
       }
  } while (pdev != NULL);


  pdev = NULL;
  /* check for any AHCI - SATA Controllers */
  do {
  
       pdev = pal_pci_get_dev(SATA_CLASSCODE, pdev);
       if (pdev != 0) {
          per_info->type  = PERIPHERAL_TYPE_SATA;
          per_info->base0 = pal_pcie_get_base(pdev, BAR0);
          per_info->base1 = pal_pcie_get_base(pdev, BAR5);
          per_info->bdf   = pal_pcie_get_bdf(pdev);
          per_info->flags = pci_dev_msi_enabled(pdev)? PER_FLAG_MSI_ENABLED : 0;
          per_info->irq   = pdev->irq;
          peripheralInfoTable->header.num_sata++;
          per_info++;
       }

  } while (pdev != NULL);
  
  per_info->type = 0xFF; //indicate end of table

}


unsigned long long
pal_memory_ioremap(void *addr, uint32_t size, uint32_t attr)
{

    return ((unsigned long long)ioremap_wc((phys_addr_t)addr, size));

}

void
pal_memory_unmap(void *addr)
{

  iounmap((char *)addr);
}
