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

#include <linux/slab.h>
#include <asm/io.h>
#include "include/pal_linux.h"

unsigned int *gSharedMemory;

int tail_msg = 0;
int num_msg = MIN_NUM_MSG;
/**
  @brief  Provides a single point of abstraction to read from all 
          Memory Mapped IO address

  @param  addr 64-bit address

  @return 32-bit data read from the input address
**/
uint32_t
pal_mmio_read(uint64_t addr)
{
  uint32_t data;
  uint32_t *p;

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
  @brief  Provides a single point of abstraction to write to all 
          Memory Mapped IO address

  @param  addr  64-bit address
  @param  data  32-bit data to write to address

  @return None
**/
void
pal_mmio_write(uint64_t addr, uint32_t data)
{
  uint32_t *p;
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
void
pal_print(char *string, uint64_t data)
{
  char buf[MSG_SIZE], *tmp;

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
  sprintf(buf, string, data);
  memcpy(g_msg_buf+(tail_msg*MSG_SIZE), buf, sizeof(buf));
  tail_msg = tail_msg+1;

}

/**
  @brief this function is irrelevant for linux code
**/
void
pal_print_raw(uint64_t addr, char *string, uint64_t data)
{
}

/**
  @brief  Free the memory allocated by UEFI Framework APIs
  @param  Buffer the base address of the memory range to be freed

  @return None
**/
void
pal_mem_free(void *buffer)
{

  kfree(buffer);
}

/**
  @brief  Allocate memory which is to be used to share data across PEs

  @param  num_pe      - Number of PEs in the system
  @param  sizeofentry - Size of memory region allocated to each PE

  @return None
**/
void
pal_mem_allocate_shared(unsigned int num_pe, unsigned int sizeofentry)
{
  gSharedMemory = 0; 

  gSharedMemory = kmalloc((num_pe * sizeofentry), GFP_KERNEL);

  return;

}

/**
  @brief  Return the base address of the shared memory region to the VAL layer
**/
uint64_t
pal_mem_get_shared_addr(void)
{

  return (uint64_t) (gSharedMemory);
}

/**
  @brief  Free the shared memory region allocated above
**/
void
pal_mem_free_shared(void)
{
  kfree ((void *)gSharedMemory);
}
