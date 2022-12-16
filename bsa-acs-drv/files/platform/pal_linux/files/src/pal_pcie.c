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
 * Copyright (C) 2016-2022 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *         Daniil Egranov <daniil.egranov@arm.com>
 *
 */

#include "include/pal_linux.h"
#include "include/pal_dt.h"

#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/acpi.h>
#include <linux/version.h>
#include <linux/pci-acpi.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

/**
    @brief   Read a device MSI(X) vector

    @param   dev       pointer to a pci_dev device structure
    @param   entry     pointer to a msi_desc structure assosiated with a device
    @param   vector    pointer to a MSI(X) vector structure

    @return  vector    MSI(X) vector
**/

static
uint32_t irq_to_hwirq(uint32_t irq)
{
  struct irq_data *d = irq_get_irq_data(irq);
  if (d)
      return d->hwirq;
  return 0;
}

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
  vector->vector_irq_base = irq_to_hwirq(entry->irq);
  vector->vector_mapped_irq_base = entry->irq;
  vector->vector_n_irqs = entry->nvec_used;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,16,0)
  if (entry->pci.msi_attrib.is_msix) {
    base = entry->pci.mask_base +
        entry->msi_index * PCI_MSIX_ENTRY_SIZE;
#else
  if (entry->msi_attrib.is_msix) {
    base = entry->mask_base +
        entry->msi_attrib.entry_nr * PCI_MSIX_ENTRY_SIZE;
#endif
    vector->vector_lower_addr = readl (base + PCI_MSIX_ENTRY_LOWER_ADDR);
    vector->vector_upper_addr = readl (base + PCI_MSIX_ENTRY_UPPER_ADDR);
    vector->vector_data = readl (base + PCI_MSIX_ENTRY_DATA);
    vector->vector_control = readl (base + PCI_MSIX_ENTRY_VECTOR_CTRL);
  } else {
    pos = dev->msi_cap;
    pci_read_config_dword (dev, pos + PCI_MSI_ADDRESS_LO,
              &vector->vector_lower_addr);
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,16,0)
    if (entry->pci.msi_attrib.is_64) {
#else
    if (entry->msi_attrib.is_64) {
#endif
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
pal_get_msi_vectors (uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn, PERIPHERAL_VECTOR_LIST **mvector)
{
  struct pci_dev *pdev;
  struct msi_desc *entry;
  uint32_t vcount;
  PERIPHERAL_VECTOR_LIST *head;

  vcount = 0;
  head = NULL;

  if(mvector == NULL)
    return 0;

  pdev = pci_get_domain_bus_and_slot (seg, bus, PCI_DEVFN (dev, fn));

  if(pdev != NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,16,0)
    msi_lock_descs(&pdev->dev);
    msi_for_each_desc(entry, &pdev->dev, MSI_DESC_ALL) {
#else
    for_each_pci_msi_entry (entry, pdev) {
#endif
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(5,16,0)
    msi_unlock_descs(&pdev->dev);
#endif
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

    if (mcfg) {
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

    } else {
        pal_pcie_create_info_table_dt(PcieTable);
    }

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
pal_pcie_get_legacy_irq_map(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn, PERIPHERAL_IRQ_MAP *irq_map)
{
  acpi_status status;
  struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
  struct acpi_pci_routing_table *entry;
  acpi_handle handle = NULL;
  struct pci_dev *pdev;
  uint32_t irq_count;

  /* Get a root bridge device */
  pdev = pci_get_domain_bus_and_slot (seg, bus, PCI_DEVFN (dev, fn));
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
pal_pcie_is_device_behind_smmu(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev->dev.iommu_group)
      return 1;
  else
      return 0;
}

/**
    @brief   Return the DMA addressability of the device

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  DMA Mask : 0, 0xffffffff or 0xffffffffffff
**/
uint32_t
pal_pcie_is_devicedma_64bit(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  if (pdev) {
      bsa_print(ACS_PRINT_INFO,"dma mask is %llx \n", *pdev->dev.dma_mask);
      if (*pdev->dev.dma_mask == DMA_BIT_MASK(64))
          return 1;
      else
          return 0;
  }
  return 0;
}

/**
    @brief   Return if driver present for a pcie device

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  Driver present or not : 0, 1
**/
uint32_t
pal_pcie_device_driver_present(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  struct pci_driver *driver;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

  if (pdev) {
      driver = (*pdev).driver;
      if (driver && driver->name) {
          bsa_print(ACS_PRINT_INFO,"driver name is %s \n", *driver->name);
          return 1;
      }
      else
          return 0;
  }
  return 0;
}

/**
    @brief   This API scans bridge devices and checks memory type

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   seg        PCI segment number

    @return  0 -> 32-bit mem type, 1 -> 64-bit mem type
**/
uint32_t pal_pcie_scan_bridge_devices_and_check_memtype(uint32_t seg, uint32_t bus,
                                                        uint32_t dev,uint32_t fn)
{
    struct pci_dev *pdev;
    struct pci_dev *child_dev;
    uint16_t data = 0;
    uint32_t status = 0;
    uint8_t mem_type;

    pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));

    list_for_each_entry(child_dev, &pdev->subordinate->devices, bus_list) {
        if (child_dev) {
            pci_read_config_word(child_dev, 0x10, &data);
            if (data) {
                mem_type = data & 0x6;
                if (mem_type != 0) {
                    status = 1;
                    break;
                }
            }
        }
    }
    return status;
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
  struct pci_dev *pdev, *root_port = NULL;
  pdev = pci_get_domain_bus_and_slot(*seg, *bus, PCI_DEVFN(*dev, *func));
  if(pdev->bus->self == NULL)
    return 1;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5,7,0)
  root_port = pcie_find_root_port(pdev);
#else
  root_port = pci_find_pcie_root_port(pdev);
#endif

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
pal_pcie_get_device_type(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  u16 class;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 0;
  class = pdev->class >> 8;

  if(class == PCI_CLASS_BRIDGE_HOST)
	return 2;
  if(!pci_is_bridge(pdev))
    return 1;
  else
    return 3;
}

/**
    @brief   Get the PCIe device/port type

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number

    @return  Returns PCIe device/port type
**/
uint32_t
pal_pcie_get_pcie_type(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 0;

 return pci_pcie_type(pdev);
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
pal_pcie_get_snoop_bit(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  u16 devctl_cap;
  uint32_t ret_val;
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
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
pal_pcie_get_dma_support(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  uint32_t ret_val;
  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
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
pal_pcie_get_dma_coherent(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;
  enum dev_dma_attr dma_attr;
  uint32_t ret_val;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
    return 2;

  ret_val = 0;
  dma_attr = device_get_dma_attr(&pdev->dev);
  if (dma_attr == DEV_DMA_COHERENT) {
    ret_val = 1;
  }

  return ret_val;
}

void
pal_pcie_read_ext_cap_word(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn,
                           uint32_t ext_cap_id, uint8_t offset, uint16_t *val)
{
  struct pci_dev *pdev;
  int pos;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL) {
      *val = 0;
      return;
  }

  pos = pci_find_ext_capability(pdev, ext_cap_id);
  if (!pos) {
      *val = 0;
      return;
  }

  pci_read_config_word(pdev, pos + offset, val);
}

/**
  @brief   This API checks the PCIe Hierarchy Supports P2P
           1. Caller       -  Test Suite
  @return  1 - P2P feature not supported 0 - P2P feature supported
**/
uint32_t
pal_pcie_p2p_support(void)
{
  /*
   * TODO
   * PCIe support for peer to peer
   * transactions is platform implementation specific
   */

  return 0;
}

/**
  @brief   This API checks the PCIe device P2P support
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  1 - P2P feature not supported 0 - P2P feature supported
**/
uint32_t
pal_pcie_dev_p2p_support(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  /*
   * TODO
   * Root port or Switch support for peer to peer
   * transactions is platform implementation specific
   */

  return 1;
}

/**
  @brief   This API checks the PCIe device multifunction support
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  1 - Multifunction feature not supported 0 - Multifunction feature supported
**/
uint32_t
pal_pcie_multifunction_support(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  struct pci_dev *pdev;

  pdev = pci_get_domain_bus_and_slot(seg, bus, PCI_DEVFN(dev, fn));
  if(pdev == NULL)
      return 1;

  if (!pdev->multifunction)
      return 1;

  return 0;
}

uint32_t pal_pcie_io_read_cfg(uint32_t bdf, uint32_t offset, uint32_t *data)
{
    return 0;
}

/**
  @brief  Placeholder To Perform PCIe Write
 **/
void pal_pcie_io_write_cfg(uint32_t bdf, uint32_t offset, uint32_t data)
{

}

/**
  @brief  Placeholder To Perform PCIe Enumeration
 **/
void pal_pcie_enumerate(void)
{

}

/**
  @brief  Placeholder To Perform PCIe Enumeration
 **/

uint32_t pal_bsa_pcie_enumerate(void)
{
    return 0;
}

/**
  @brief  Placeholder To check if a PCIe device has an Address
          translation cache or not.
  @return  1 - ATC not supported 0 - ATC supported

 **/
uint32_t
pal_pcie_is_cache_present(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  return 1;
}

/**
    @brief   Gets RP support of transaction forwarding.

    @param   bus        PCI bus address
    @param   dev        PCI device address
    @param   fn         PCI function number
    @param   seg        PCI segment number

    @return  0 if rp not involved in transaction forwarding
             1 if rp is involved in transaction forwarding
**/
uint32_t
pal_pcie_get_rp_transaction_frwd_support(uint32_t seg, uint32_t bus, uint32_t dev, uint32_t fn)
{
  return 1;
}

/**
  @brief  Returns whether a PCIe Function is an on-chip peripheral or not

  @param  bdf        - Segment/Bus/Dev/Func in the format of PCIE_CREATE_BDF
  @return Returns 1 if the Function is on-chip peripheral, 0 if it is
          not an on-chip peripheral
**/
uint32_t
pal_pcie_is_onchip_peripheral(uint32_t bdf)
{
  return 0;
}

/**
  @brief  Checks the discovered PCIe hierarchy is matching with the
          topology described in info table.
  @return Returns 0 if device entries matches , 1 if there is mismatch.
**/
uint32_t
pal_pcie_check_device_list()
{
  return 0;
}

/**
  @brief  Placeholder to return the memory offset
          This offset is platform-specific. It needs to
          be modified according to the requirement.

**/
uint32_t
pal_pcie_mem_get_offset(uint32_t type)
{

  return 0;
}
