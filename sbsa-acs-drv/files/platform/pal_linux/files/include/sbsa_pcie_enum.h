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

#ifndef __SBSA_PCIE_ENUM_H__
#define __SBSA_PCIE_ENUM_H__


#define PCIE_EXTRACT_BDF_SEG(bdf)  ((bdf >> 24) & 0xFF)
#define PCIE_EXTRACT_BDF_BUS(bdf)  ((bdf >> 16) & 0xFF)
#define PCIE_EXTRACT_BDF_DEV(bdf)  ((bdf >> 8) & 0xFF)
#define PCIE_EXTRACT_BDF_FUNC(bdf) (bdf & 0xFF)

#define PCIE_CREATE_BDF(Seg, Bus, Dev, Func) ((Seg << 24) | (Bus << 16) | (Dev << 8) | Func)


struct pci_dev *
pal_pci_get_dev(unsigned int class_code, struct pci_dev *dev);

struct pci_dev *
pal_pci_get_dev_next (struct pci_dev *from_dev);

unsigned long
pal_pcie_get_base(struct pci_dev *dev, unsigned int bar_index);

unsigned int
pal_pcie_get_bdf(struct pci_dev *dev);

unsigned int
pal_pcie_get_bdf_wrapper(unsigned int class_code, unsigned int start_bdf);

void *
pal_pci_bdf_to_dev(unsigned int bdf);

void
pal_pci_read_config_byte(uint32_t bdf, uint8_t offset, uint8_t *val);

void
pal_pci_write_config_byte(uint32_t bdf, uint8_t offset, uint8_t val);

#endif
