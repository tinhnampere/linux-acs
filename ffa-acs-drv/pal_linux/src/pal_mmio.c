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

#include <linux/slab.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include "pal_interfaces.h"
#include "pal_mmio.h"

/* MMIO read/write access functions */
/**
  @brief  Provides a single point of abstraction to read 8 bit data from
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 8-bit data read from the input address
**/
uint8_t pal_mmio_read8(uint64_t addr)
{
  uint8_t data;
  void __iomem *p;

  p = ioremap(addr, 16);
  data = ioread8(p);

  iounmap(p);

  return data;
}

/**
  @brief  Provides a single point of abstraction to read 16 bit data from
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 16-bit data read from the input address
**/
uint16_t pal_mmio_read16(uint64_t addr)
{
  uint16_t data;
  void __iomem *p;

  p = ioremap(addr, 16);
  data = ioread16(p);

  iounmap(p);

  return data;
}

/**
  @brief  Provides a single point of abstraction to read 64 bit data from
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 64-bit data read from the input address
**/
uint64_t pal_mmio_read64(uint64_t addr)
{
  uint64_t data;
  void __iomem *p;

  p = ioremap(addr, 16);
  data = ioread64(p);

  iounmap(p);

  return data;
}

/**
  @brief  Provides a single point of abstraction to read 32-bit data from
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 32-bit data read from the input address
**/
uint32_t pal_mmio_read32(uint64_t addr)
{
  uint32_t data;
  void __iomem *p;

  if (addr & 0x3) {
      addr = addr & ~(0x3);  //make sure addr is aligned to 4 bytes
  }
  p = ioremap(addr, 16);
  //Print(L"Address = %8x  ", addr);
  data = ioread32(p);

  //Print(L" data = %8x \n", data);
  iounmap(p);

  return data;
}

/**
  @brief  Provides a single point of abstraction to write 8-bit data to
          Memory Mapped IO address

  @param  addr  64-bit address
  @param  data  8-bit data to write to address

  @return None
**/
void pal_mmio_write8(uint64_t addr, uint8_t data)
{
  void __iomem *p;
  p = ioremap(addr, 16);
  //Print(L"Address = %8x  Data = %8x \n", addr, data);
  iowrite8(data, p);

  iounmap(p);
}

/**
  @brief  Provides a single point of abstraction to write 16-bit data to
          Memory Mapped IO address

  @param  addr  64-bit address
  @param  data  16-bit data to write to address

  @return None
**/
void pal_mmio_write16(uint64_t addr, uint16_t data)
{
  void __iomem *p;
  p = ioremap(addr, 16);
  //Print(L"Address = %8x  Data = %8x \n", addr, data);
  iowrite16(data, p);

  iounmap(p);
}

/**
  @brief  Provides a single point of abstraction to write 64-bit data to
          Memory Mapped IO address

  @param  addr  64-bit address
  @param  data  64-bit data to write to address

  @return None
**/
void pal_mmio_write64(uint64_t addr, uint64_t data)
{
  void __iomem *p;
  p = ioremap(addr, 16);
  //Print(L"Address = %16x  Data = %16x \n", addr, data);
  iowrite64(data, p);

  iounmap(p);
}

/**
  @brief  Provides a single point of abstraction to write 32-bit data to
          Memory Mapped IO address

  @param  addr  64-bit address
  @param  data  32-bit data to write to address

  @return None
**/
void pal_mmio_write32(uint64_t addr, uint32_t data)
{
  void __iomem *p;
  p = ioremap(addr, 16);
  //Print(L"Address = %8x  Data = %8x \n", addr, data);
  iowrite32(data, p);

  iounmap(p);
}
