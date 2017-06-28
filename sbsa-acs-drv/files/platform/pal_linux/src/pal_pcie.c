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
 *         Daniil Egranov <daniil.egranov@arm.com>
 *
 */

#include "include/pal_linux.h"

#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/acpi.h>
#include <linux/pci-acpi.h>
#include <linux/interrupt.h>

/**
    @brief   Read a device MSI(X) vector

    @param   dev       pointer to a pci_dev device structure
    @param   entry     pointer to a msi_desc structure assosiated with a device
    @param   vector    pointer to a MSI(X) vector structure

    @return  vector    MSI(X) vector
**/
static
void
pal_pci_read_msi_vector (struct pci_dev *dev, struct msi_desc *entry, PERIPHERAL_VECTOR_BLOCK *vector)
{
  int pos;
  uint16_t data;
  void __iomem *base;

  if(dev == NULL || entry == NULL || vector == NULL) {
    return;
  }

  vector->vector_lower_addr = 0;
  vector->vector_upper_addr = 0;
  vector->vector_data = 0;
  vector->vector_control = 0;
  vector->vector_irq_base = entry->irq;

  if (entry->msi_attrib.is_msix) {
    base = entry->mask_base +
        entry->msi_attrib.entry_nr * PCI_MSIX_ENTRY_SIZE;

    vector->vector_lower_addr = readl (base + PCI_MSIX_ENTRY_LOWER_ADDR);
    vector->vector_upper_addr = readl (base + PCI_MSIX_ENTRY_UPPER_ADDR);
    vector->vector_data = readl (base + PCI_MSIX_ENTRY_DATA);
    vector->vector_control = readl (base + PCI_MSIX_ENTRY_VECTOR_CTRL);
  } else {
    pos = dev->msi_cap;
    pci_read_config_dword (dev, pos + PCI_MSI_ADDRESS_LO,
              &vector->vector_lower_addr);
    if (entry->msi_attrib.is_64) {
      pci_read_config_dword (dev, pos + PCI_MSI_ADDRESS_HI,
                &vector->vector_upper_addr);
      pci_read_config_word (dev, pos + PCI_MSI_DATA_64, &data);
    } else {
      pci_read_config_word (dev, pos + PCI_MSI_DATA_32, &data);
    }
    vector->vector_data = data;
  }
}

/**
    @brief   Create a list of MSI(X) vectors for a device

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   mvector    pointer to a MSI(X) list address

    @return  mvector    list of MSI(X) vectors
    @return  number of MSI(X) vectors
**/
uint32_t
pal_get_msi_vectors (uint32_t bus, uint32_t dev, uint32_t fn, PERIPHERAL_VECTOR_LIST **mvector)
{
  struct pci_dev *pdev;
  struct msi_desc *entry;
  uint32_t vcount;
  PERIPHERAL_VECTOR_LIST *head;

  vcount = 0;
  head = NULL;

  if(mvector == NULL)
    return 0;

  pdev = pci_get_bus_and_slot (bus, PCI_DEVFN (dev, fn));

  if(pdev != NULL) {
    for_each_pci_msi_entry (entry, pdev) {
      if (head == NULL) {
        head = kmalloc (sizeof (PERIPHERAL_VECTOR_LIST), GFP_KERNEL);
        if(head == NULL) {
          break;
        }
        head->next = NULL;
        *mvector = head;
      } else {
        head->next = kmalloc (sizeof (PERIPHERAL_VECTOR_LIST), GFP_KERNEL);
        if(head->next == NULL) {
          break;
        }
        head = head->next;
        head->next = NULL;
      }
      pal_pci_read_msi_vector (pdev, entry, &head->vector);
      vcount++;
    }
  }

  return vcount;
}

uint64_t
pal_pcie_get_mcfg_ecam(void)
{
    struct acpi_table_mcfg      *mcfg;
    struct acpi_mcfg_allocation *cfg_table;

    mcfg = (struct acpi_table_mcfg *)pal_get_mcfg_ptr();

    if (!mcfg)
        return 0;

    cfg_table = (struct acpi_mcfg_allocation *) &mcfg[1];

    if (cfg_table)
        return cfg_table->address;
    else
        return 0;
}


/**
  @brief  Fill the PCIE Info table with the details of the PCIe sub-system
 **/
void
pal_pcie_create_info_table(PCIE_INFO_TABLE *PcieTable)
{
    unsigned int i = 0, length;
    struct acpi_table_mcfg      *mcfg;
    struct acpi_mcfg_allocation *entry;

    length = sizeof(struct acpi_table_mcfg);

    PcieTable->num_entries = 0;
    mcfg = (struct acpi_table_mcfg *)pal_get_mcfg_ptr();

    if (!mcfg)
        return;

    entry = (struct acpi_mcfg_allocation *) &mcfg[1];

    do{
        PcieTable->block[i].ecam_base = entry->address;
        PcieTable->block[i].start_bus_num = entry->start_bus_number;
        PcieTable->block[i].end_bus_num = entry->end_bus_number;
        PcieTable->block[i].segment_num = entry->pci_segment;
        length += sizeof(struct acpi_mcfg_allocation);
        entry++;
        i++;
        PcieTable->num_entries++;
    } while((length < mcfg->header.length) && (entry));

    return;
}

/**
    @brief   Get legacy IRQ routing for a PCI device

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   irq_map    pointer to IRQ map structure

    @return  irq_map    IRQ routing map
    @return  staus code
**/

uint32_t
pal_pcie_get_legacy_irq_map(uint32_t bus, uint32_t dev, uint32_t fn, PERIPHERAL_IRQ_MAP *irq_map)
{
  acpi_status status;
  struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
  struct acpi_pci_routing_table *entry;
  acpi_handle handle = NULL;
  struct pci_dev *pdev;
  uint32_t irq_count;

  /* Get a root bridge device */
  pdev = pci_get_bus_and_slot (bus, PCI_DEVFN (dev, fn));
  if (pdev == NULL || !pdev->bus->bridge) {
    return 1;
  }

  /* Get handle for _PRT */
  handle = ACPI_HANDLE (pdev->bus->bridge);
  if (!handle) {
    return 2;
  }

  /* Get routing irq data from _PRT  */
  status = acpi_get_irq_routing_table(handle, &buffer);
  if (ACPI_FAILURE(status)) {
    kfree(buffer.pointer);
    return 3;
  }

  entry = buffer.pointer;
  status = 0;
  while (entry && (entry->length > 0)) {

    /*
     * The 'source' field expected to be NULL, and the 'source_index'
     * field specifies the IRQ value, which is hard-wired to specific interrupt inputs
     * on the interrupt controller.
     */
    if ((uint32_t)*entry->source != 0) {
      status = 4;
      break;
    }

    /*
     * Expecting pins A, B, C and D
     */
    if (entry->pin > 3) {
        status = 5;
        break;
    }

    irq_count = irq_map->legacy_irq_map[entry->pin].irq_count;
    if (irq_count >= MAX_IRQ_CNT) {
      status = 6;
      break;
    } else {
      irq_map->legacy_irq_map[entry->pin].irq_list[irq_count] = entry->source_index;
      irq_map->legacy_irq_map[entry->pin].irq_count++;
    }

    entry = (struct acpi_pci_routing_table *) ((unsigned long)entry + entry->length);
  }

  kfree(buffer.pointer);
  return status;
}

/**
    @brief   Checks if device is behind SMMU

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  staus code:0 -> not present, nonzero -> present
**/
uint32_t
pal_pcie_is_device_behind_smmu(uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  pdev = pci_get_bus_and_slot(bus, PCI_DEVFN(dev, fn));
  if(pdev->dev.iommu_group)
      return 1;
  else
      return 0;
}

/**
    @brief   Get bdf of root port

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   seg        PCI segment number

    @return  BDF of root port
    @return  staus code
**/
uint32_t
pal_pcie_get_root_port_bdf(uint32_t *seg, uint32_t *bus, uint32_t *dev, uint32_t *func)
{
  struct pci_dev *pdev, *root_port;
  pdev = pci_get_bus_and_slot(*bus, PCI_DEVFN(*dev, *func));
  if(pdev->bus->self == NULL)
    return 1;

  root_port = pci_find_pcie_root_port(pdev);
  if(root_port == NULL)
    return 2;

  *bus  = root_port->bus->number;
  *dev  = PCI_SLOT(root_port->devfn);
  *func = PCI_FUNC(root_port->devfn);
  *seg  = pci_domain_nr(root_port->bus);
  return 0;
}
/**
    @brief   Get the PCIe device type

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  staus code:
             1: Normal PCIe device, 2: PCIe Host bridge,
             3: PCIe bridge device, else: INVALID
**/
uint32_t
pal_pcie_get_device_type(uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  pdev = pci_get_bus_and_slot(bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 0;

  if(!pci_is_bridge(pdev))
    return 1;
  else{
    if(pci_upstream_bridge(pdev) == NULL)
      return 2;
    else
      return 3;
  }
}

/**
    @brief   Get the PCIe device snoop bit transaction attribute

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  0 snoop
             1 no snoop
             2 device error
**/
uint32_t
pal_pcie_get_snoop_bit(uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  u16 devctl_cap;
  uint32_t ret_val;
  pdev = pci_get_bus_and_slot(bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 2;

  pcie_capability_read_word(pdev, PCI_EXP_DEVCTL, &devctl_cap);
  /* Extract bit 11 (no snoop) */
  ret_val = (devctl_cap >> DEVCTL_SNOOP_BIT) & 0x1;

  return ret_val;
}

/**
    @brief   Get the PCIe device DMA support

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  0 no support
             1 support
             2 device error
**/
uint32_t
pal_pcie_get_dma_support(uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  uint32_t ret_val;
  pdev = pci_get_bus_and_slot(bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 2;

  ret_val = device_dma_supported(&pdev->dev);

  return ret_val;
}

/**
    @brief   Get the PCIe device DMA coherency support

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  0 DMA is not coherent
             1 DMA is coherent
             2 device error
**/
uint32_t
pal_pcie_get_dma_coherent(uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  enum dev_dma_attr dma_attr;
  uint32_t ret_val;

  pdev = pci_get_bus_and_slot(bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 2;

  ret_val = 0;
  dma_attr = device_get_dma_attr(&pdev->dev);
  if (dma_attr == DEV_DMA_COHERENT) {
    ret_val = 1;
  }

  return ret_val;
}
