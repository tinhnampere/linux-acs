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
 * Copyright (C) 2016-2018, 2022 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/init.h>
#include <linux/pci.h>

#include "include/sbsa_pcie_enum.h"
#include "include/pal_linux.h"


/**
    @brief   Returns the Bus, Dev, Function (in the form seg<<24 | bus<<16 | Dev <<8 | func)
             for a matching class code.

    @param   ClassCode  - is a 32bit value of format ClassCode << 16 | sub_class_code
    @param   StartBdf   - is 0     : start enumeration from Host bridge
                          is not 0 : start enumeration from the input segment, bus, dev
                          this is needed as multiple controllers with same class code are
                          potentially present in a system.
    @return  the BDF of the device matching the class code
**/
struct pci_dev *
pal_pci_get_dev(unsigned int class_code, struct pci_dev *dev)
{

  return pci_get_class(class_code, dev);
}

/**
    @brief   Returns next PCI device

    @param   from_dev    previous PCI device or NULL

    @return  Next PCI device or NULL
**/
struct pci_dev *
pal_pci_get_dev_next (struct pci_dev *from_dev)
{

  return pci_get_device (PCI_ANY_ID, PCI_ANY_ID, from_dev);
}

/**
  @brief  This API returns the Base Address Register value for a given BDF and index
  @param  bdf       - the device whose PCI Config space BAR needs to be returned.
  @param  bar_index - the '0' based BAR index identifying the correct 64-bit BAR

  @return the 64-bit BAR value
*/
unsigned long
pal_pcie_get_base(struct pci_dev *dev, unsigned int bar_index)
{

  return dev->resource[bar_index].start;
}

unsigned int
pal_pcie_get_bdf(struct pci_dev *dev)
{

  return (PCIE_CREATE_BDF(pci_domain_nr(dev->bus), dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn)));
}

unsigned int
pal_pcie_get_bdf_wrapper(unsigned int class_code, unsigned int bdf)
{
  uint32_t seg;
  uint32_t bus;
  uint32_t dev;
  uint32_t fn;
  struct pci_dev *pdev;

  pdev = NULL;
  if (bdf) {
      seg  = PCIE_EXTRACT_BDF_SEG (bdf);
      bus = PCIE_EXTRACT_BDF_BUS(bdf);
      dev = PCIE_EXTRACT_BDF_DEV(bdf);
      fn = PCIE_EXTRACT_BDF_FUNC(bdf);
      pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  }

  pdev = pal_pci_get_dev(class_code, pdev);
  return pal_pcie_get_bdf(pdev);
}

void *
pal_pci_bdf_to_dev(unsigned int bdf)
{
  uint32_t seg;
  uint32_t bus;
  uint32_t dev;
  uint32_t fn;
  struct pci_dev *pdev;

  seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  bus = PCIE_EXTRACT_BDF_BUS(bdf);
  dev = PCIE_EXTRACT_BDF_DEV(bdf);
  fn = PCIE_EXTRACT_BDF_FUNC(bdf);
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  if (pdev)
      return ((void *)(&pdev->dev));

  return NULL;
}

void pal_pci_read_config_byte(uint32_t bdf, uint8_t offset, uint8_t *val)
{
  uint32_t seg;
  uint32_t bus;
  uint32_t dev;
  uint32_t fn;
  struct pci_dev *pdev;

  seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  bus = PCIE_EXTRACT_BDF_BUS(bdf);
  dev = PCIE_EXTRACT_BDF_DEV(bdf);
  fn = PCIE_EXTRACT_BDF_FUNC(bdf);
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  pci_read_config_byte(pdev, offset, val);
}

void pal_pci_write_config_byte(uint32_t bdf, uint8_t offset, uint8_t val)
{
  uint32_t seg;
  uint32_t bus;
  uint32_t dev;
  uint32_t fn;
  struct pci_dev *pdev;

  seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  bus = PCIE_EXTRACT_BDF_BUS(bdf);
  dev = PCIE_EXTRACT_BDF_DEV(bdf);
  fn = PCIE_EXTRACT_BDF_FUNC(bdf);
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  pci_write_config_byte(pdev, offset, val);
}

/**
  @brief  This API is checks if the bdf obtained
          is valid or not

  @param  bdf
  @return 0 if bdf is valid else 1
**/

unsigned int
pal_pcie_check_device_valid(unsigned int bdf)
{
  uint32_t seg;
  uint32_t bus;
  uint32_t dev;
  uint32_t fn;
  struct pci_dev *pdev;

  seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  bus = PCIE_EXTRACT_BDF_BUS(bdf);
  dev = PCIE_EXTRACT_BDF_DEV(bdf);
  fn = PCIE_EXTRACT_BDF_FUNC(bdf);
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  if (pdev)
      return 0;

  return 1;

}
