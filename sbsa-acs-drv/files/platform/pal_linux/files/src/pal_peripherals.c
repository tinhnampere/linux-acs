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
 * Copyright (C) 2016-2018 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci-ats.h>

#include "include/pal_linux.h"
#include "include/sbsa_pcie_enum.h"
//#include "include/platform_override.h"

#define USB_CLASSCODE   0x0C0330
#define SATA_CLASSCODE  0x010601
#define BAR0            0
#define BAR1            1
#define BAR2            2
#define BAR3            3
#define BAR4            4
#define BAR5            5

void
pal_peripheral_create_dma_table(void);

void
pal_peripheral_create_info_table(PERIPHERAL_INFO_TABLE *peripheralInfoTable)
{

  struct pci_dev *pdev = NULL;
  int max_pasids;
  PERIPHERAL_INFO_BLOCK *per_info;

  per_info = peripheralInfoTable->info;

  peripheralInfoTable->header.num_usb = 0;
  peripheralInfoTable->header.num_sata = 0;
  peripheralInfoTable->header.num_uart = 0;
  peripheralInfoTable->header.num_all = 0;

  /* Collect all PCI devices */
  do {
       pdev = pal_pci_get_dev_next (pdev);
       if (pdev != NULL) {
         per_info->base0 = pal_pcie_get_base (pdev, BAR0);
         per_info->bdf = pal_pcie_get_bdf (pdev);
         per_info->msi = pdev->msi_cap;
         per_info->msix = pdev->msix_cap;
         if (per_info->msi || per_info->msix)
            per_info->flags = PER_FLAG_MSI_ENABLED;
         else
             per_info->flags = 0;
         per_info->irq = pci_irq_vector(pdev, 0);
         max_pasids = pci_max_pasids(pdev);
         per_info->max_pasids = (max_pasids < 0)?0:max_pasids;
         peripheralInfoTable->header.num_all++;

         switch (pdev->class) {
          /* check for any XHCI - USB Controllers */
          case USB_CLASSCODE:
            per_info->type  = PERIPHERAL_TYPE_USB;
            sbsa_print(AVS_PRINT_INFO, "Found a USB controller %4x \n", per_info->bdf);
            peripheralInfoTable->header.num_usb++;
            break;
          /* check for any AHCI - SATA Controllers */
          case SATA_CLASSCODE:
            per_info->type  = PERIPHERAL_TYPE_SATA;
            sbsa_print(AVS_PRINT_INFO, "Found a SATA controller %4x \n", per_info->bdf);
            per_info->base1 = pal_pcie_get_base (pdev, BAR5);
            peripheralInfoTable->header.num_sata++;
            break;
          default:
            per_info->type  = PERIPHERAL_TYPE_OTHER;
            break;
         }
         per_info++;
       }
  } while (pdev != NULL);

  per_info->type = 0xFF; //indicate end of table

}

/**
    @brief   Check if PCI device is PCI Express capable

    @param   seg        PCI segment number
    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  staus code:
             1: PCIe capable,  0: No PCIe capable
**/
uint32_t pal_peripheral_is_pcie(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
    struct pci_dev *pdev;

    pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
    if (pdev == NULL)
        return 0;

    if (pci_is_pcie(pdev))
        return 1;
    else
        return 0;
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
