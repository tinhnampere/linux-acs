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
 * Copyright (C) 2016-2020 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/slab.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include "include/pal_linux.h"

unsigned int *gSharedMemory;

extern int tail_msg;
int num_msg = MIN_NUM_MSG;

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

  //Print(L"Address = %8x  ", addr);
  //Print(L" data = %8x \n", data);
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

  //Print(L"Address = %8x  ", addr);
  //Print(L" data = %8x \n", data);
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

  //Print(L"Address = %16x  ", addr);
  //Print(L" data = %16x \n", data);
  iounmap(p);

  return data;
}

/**
  @brief  Provides a single point of abstraction to read 32-bit data from
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 32-bit data read from the input address
**/
uint32_t pal_mmio_read(uint64_t addr)
{
  uint32_t data;
  void __iomem *p;

  if (addr & 0x3) {
      sbsa_print(AVS_PRINT_INFO, "\n  Error-Input address is not aligned. Masking the last 2 bits \n");
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
void pal_mmio_write(uint64_t addr, uint32_t data)
{
  void __iomem *p;
  p = ioremap(addr, 16);
  //Print(L"Address = %8x  Data = %8x \n", addr, data);
  iowrite32(data, p);

  iounmap(p);
}

/**
  @brief  Sends a formatted string to the output console

  @param  string  An ASCII string
  @param  data    data for the formatted output

  @return None
**/
void pal_print(char *string, uint64_t data)
{
  char buf[MSG_SIZE], *tmp=NULL;

  if(tail_msg >= num_msg) {
    tmp = kmalloc(NUM_MSG_GROW(num_msg) * sizeof(pal_msg_parms_t), GFP_KERNEL);
    if(tmp) {
      memcpy(tmp, g_msg_buf, num_msg * sizeof(pal_msg_parms_t));
      num_msg = NUM_MSG_GROW(num_msg);
      kfree(g_msg_buf);
      g_msg_buf = tmp;
    } else
      tail_msg = tail_msg % num_msg;
  }
  snprintf(buf, MSG_SIZE, string, data);
  memcpy(g_msg_buf+(tail_msg*MSG_SIZE), buf, sizeof(buf));
  tail_msg = tail_msg+1;

}

/**
  @brief this function is irrelevant for linux code
**/
void pal_print_raw(uint64_t addr, char *string, uint64_t data)
{
}

/**
  @brief  Allocates memory of the requested size

  @param  sizeo - Size of memory region to be allocated

  @return Virtual address if success, NULL on failure
**/
void *pal_mem_alloc(unsigned int size)
{

  return kzalloc(size, GFP_KERNEL);

}

/**
  @brief  Allocates cacheable memory of the requested size

  @param  bdf   - BDF of the requesting pcie device
  @param  size  - Size of memory region to be allocated
  @param  pa    - Physical address of the allocated memory

  @return virtual address if success, NULL on failure
**/
void *pal_mem_alloc_cacheable(uint32_t bdf, uint32_t size, void **pa)
{
  void *buf_virt;
  dma_addr_t buf_phys;
  struct device *dev;

  dev = pal_pci_bdf_to_dev(bdf);
  buf_virt = dma_alloc_coherent(dev, size, &buf_phys, GFP_KERNEL);

  *pa = (void *)buf_phys;
  return buf_virt;
}

/**
  @brief  Free the memory allocated by UEFI Framework APIs
  @param  Buffer the base address of the memory range to be freed

  @return None
**/
void pal_mem_free(void *buffer)
{

  kfree(buffer);
}

/**
  @brief  Compare the two input buffer content
  @param  src   - Source buffer to be compared
  @dest   dest  - Destination buffer to be compared

  @return Zero if buffer content are equal, else non-zero
**/
int pal_mem_compare(void *src, void *dest, uint32_t len)
{
  return memcmp(src, dest, len);
}

/**
  @brie a buffer with a known specified input value
  @param  buf   - Pointer to the buffer to fill
  @param  size  - Number of bytes in buffer to fill
  @param  value - Value to fill buffer with

  @return None
**/
void pal_mem_set(void *buf, uint32_t size, uint8_t value)
{
  memset(buf, value, size);
}

/**
  @brief  Free the cacheable memory allocated by Linux DMA Framework APIs

  @param  bdf   - BDF of the requesting pcie device
  @param  size  - Size of memory region to be freed
  @param  va    - Virtual address of the memory to be freed
  @param  pa    - Physical address of the memory to be freed

  @return None
**/
void pal_mem_free_cacheable(uint32_t bdf, uint32_t size, void *va, void *pa)
{
  struct device *dev;

  dev = pal_pci_bdf_to_dev(bdf);
  dma_free_coherent(dev, size, va, (dma_addr_t)pa);
}

/**
  @brief  Returns the physical address of the input virtual address

  @param  va    - Virtual address of the memory to be converted

  @return Physical address
**/
void *pal_mem_virt_to_phys(void *va)
{
  return (void *)virt_to_phys(va);
}

/**
  @brief  Allocate memory which is to be used to share data across PEs

  @param  num_pe      - Number of PEs in the system
  @param  sizeofentry - Size of memory region allocated to each PE

  @return None
**/
void pal_mem_allocate_shared(unsigned int num_pe, unsigned int sizeofentry)
{
  gSharedMemory = 0; 

  gSharedMemory = kmalloc((num_pe * sizeofentry), GFP_KERNEL);

  return;

}

/**
  @brief  Return the base address of the shared memory region to the VAL layer
**/
uint64_t pal_mem_get_shared_addr(void)
{

  return (uint64_t) (gSharedMemory);
}

/**
  @brief  Free the shared memory region allocated above
**/
void pal_mem_free_shared(void)
{
  kfree ((void *)gSharedMemory);
}

/**
  @brief  Place holder for Memory Copy
**/
void *pal_memcpy(void *dest_buffer, void *src_buffer, uint32_t len)
{
  return NULL;
}

/**
  @brief  Place holder for Stalling the CPU for Number of Microseconds
**/
uint64_t pal_time_delay_ms(uint64_t time_ms)
{
  return 0;
}

/**
  @brief  Place holder for Comparing two Strings
**/
uint32_t pal_strncmp(char8_t *str1, char8_t *str2, uint32_t len)
{
  return 0;
}

/**
  @brief  Place holder for returning virtual address of input physical address
**/
void *pal_mem_phys_to_virt(uint64_t pa)
{
  return NULL;
}

/**
  @brief  Place holder for returning memory page size
**/
uint32_t pal_mem_page_size(void)
{
  return 0;
}

/**
  @brief  Place holder for Allocating requested number of pages
**/
void *pal_mem_alloc_pages(uint32_t num_pages)
{
  return NULL;
}

/**
  @brief  Place holder for Freeing number of pages starting
          from a given address
**/
void pal_mem_free_pages(void *page_base, uint32_t num_pages)
{
}
