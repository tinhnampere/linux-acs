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
 * Author: Sakar Arora<sakar.arora@arm.com>
 *
 */

#include <linux/acpi_iort.h>
#include <linux/kernel.h>
#include <linux/pci.h>

#include "include/pal_linux.h"

/**
  @brief  Dump the input block
**/
static void
dump_block(IOVIRT_BLOCK *block)
{
  uint32_t i;
  NODE_DATA_MAP *map = &block->data_map[0];
  switch(block->type) {
      case ACPI_IORT_NODE_ITS_GROUP:
          sbsa_print(AVS_PRINT_INFO, "\nITS Group:\n Num ITS:%d\n", (*map).id[0]);
          for(i = 0; i < block->data.its_count; i++)
              sbsa_print(AVS_PRINT_INFO, "%d ", (*map).id[i]);
          sbsa_print(AVS_PRINT_INFO, "\n");
          return;
      case ACPI_IORT_NODE_NAMED_COMPONENT:
          sbsa_print(AVS_PRINT_INFO, "\nNamed Component:\n Device Name:%s\n", block->data.name);
          break;
      case ACPI_IORT_NODE_PCI_ROOT_COMPLEX:
          sbsa_print(AVS_PRINT_INFO, "\nRoot Complex:\n PCI segment number:%d\n", block->data.segment);
          break;
      case ACPI_IORT_NODE_SMMU:
      case ACPI_IORT_NODE_SMMU_V3:
          sbsa_print(AVS_PRINT_INFO, "\nSMMU:\n Major Rev:%d\n Base Address:0x%lld\n",
          block->data.smmu.arch_major_rev, block->data.smmu.base);
        break;
  }
  sbsa_print(AVS_PRINT_INFO, "Number of ID Mappings:%d\n", block->num_data_map);
  for(i = 0; i < block->num_data_map; i++, map++) {
      sbsa_print(AVS_PRINT_INFO, "\n input_base:0x%x\n id_count:0x%x\n output_base:0x%x\n output ref:0x%x\n",
            (*map).map.input_base, (*map).map.id_count,
            (*map).map.output_base, (*map).map.output_ref);
  }
  sbsa_print(AVS_PRINT_INFO, "\n");
}

static void
dump_iovirt_table(IOVIRT_INFO_TABLE *iort)
{
  uint32_t i;
  IOVIRT_BLOCK *block = &iort->blocks[0];
  for(i = 0; i < iort->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
    dump_block(block);
}

/**
  @brief  Check ID mappings in all blocks for any overlap of ID ranges
  @param iovirt IoVirt table
**/


static int
smmu_ctx_int_distinct(uint64_t *ctx_int, int ctx_int_cnt) {
  int i, j;
  for(i = 0; i < ctx_int_cnt - 1; i++) {
    for(j = i + 1; j < ctx_int_cnt; j++) {
      if(*((uint32_t *)&ctx_int[i]) == *((uint32_t *)&ctx_int[j]))
        return 0;
    }
  }
  return 1;
}

static void
check_mapping_overlap(IOVIRT_INFO_TABLE *iovirt)
{
    IOVIRT_BLOCK *key_block, *block, *tmp;
    NODE_DATA_MAP *key_map, *map;
    uint32_t n_key_blocks, n_blocks, n_key_maps, n_maps;
    uint32_t key_start, key_end, start, end;
    /* Starting from first block, compare each mapping with all the */
    /* mappings that follow it in the table */
    for(key_block = &iovirt->blocks[0], n_key_blocks = iovirt->num_blocks;
        n_key_blocks > 0;
        key_block = IOVIRT_NEXT_BLOCK(key_block), n_key_blocks--)
    {
        if(key_block->type == ACPI_IORT_NODE_ITS_GROUP)
            continue;
        for(key_map = &key_block->data_map[0], n_key_maps = key_block->num_data_map;
            n_key_maps > 0;
            key_map++, n_key_maps--)
        {
            key_start = (*key_map).map.output_base;
            key_end = key_start + (*key_map).map.id_count - 1;
            for(block = key_block, n_blocks = n_key_blocks;
                n_blocks > 0;
                block = IOVIRT_NEXT_BLOCK(block), n_blocks--)
            {
                if(block->type == ACPI_IORT_NODE_ITS_GROUP)
                    continue;
                n_maps = block->num_data_map;
                map = &block->data_map[0];
                if(block == key_block)
                {
                    map = key_map+1;
                    n_maps--;
                }
                for(;n_maps > 0; map++, n_maps--)
                {
                    if((*map).map.output_ref != (*key_map).map.output_ref)
                        continue;
                    start = (*map).map.output_base;
                    end = start + (*map).map.id_count - 1;
                    if((key_start >= start && key_start <= end) ||
                       (key_end >= start && key_end <= end) ||
                       (key_start < start && key_end > end))
                    {
                        tmp = ACPI_ADD_PTR(IOVIRT_BLOCK, iovirt, (*map).map.output_ref);
                        if(tmp->type == ACPI_IORT_NODE_ITS_GROUP) {
                           key_block->flags |= (1 << IOVIRT_FLAG_DEVID_OVERLAP_SHIFT);
                           block->flags |= (1 << IOVIRT_FLAG_DEVID_OVERLAP_SHIFT);
                        }
                        else
                        {
                           key_block->flags |= (1 << IOVIRT_FLAG_STRID_OVERLAP_SHIFT);
                           block->flags |= (1 << IOVIRT_FLAG_STRID_OVERLAP_SHIFT);
                        }
                    }
                }
            }
        }
    }
}

/**
  @brief Find block in IovirtTable
  @param key Block to search
  @param iovirt_table Table to in which block is to be searched
  @return offset of block, if found
          0, if block not found
**/
static uint32_t
find_block(IOVIRT_BLOCK *key, IOVIRT_INFO_TABLE *iovirt_table)
{
    IOVIRT_BLOCK *block = &iovirt_table->blocks[0];
    uint8_t *cmp_end;
    uint32_t i, cmp_size;
    for(i = 0; i < iovirt_table->num_blocks; i++, block = IOVIRT_NEXT_BLOCK(block))
    {
        cmp_end = (uint8_t*)&block->flags;
        cmp_size = cmp_end - (uint8_t*)block;
        if(key->type == block->type)
        {
            /* Compare identfiers array as well in case of ITS group */
            if(block->type == ACPI_IORT_NODE_ITS_GROUP)
                cmp_size += (block->data.its_count * sizeof(uint32_t) + sizeof(block->flags));
            if(!memcmp(key, block, cmp_size))
                return (uint8_t*)block - (uint8_t*)iovirt_table;
        }
    }
    return 0;
}

/**
  @brief  Add iovirt block for given iort node

  @param  *iort         ACPI IORT table base pointer
  @param  *iort_node    IORT node base address pointer
  @param  *iovirt_table  IO Virt Table base address pointer
  @param  **block       Pointer to IORT block base address pointer,
                        where IORT block is to be added. This is modified
                        to the next address where new IORT block
                        can be created.
  @return offset from the IO Virt table base address to the IORT block
          base address passed in **block
          OR
          offset from the IO Virt table base address to the IORT block
          base address where this block is already present in the
          table.
**/
static uint32_t
iovirt_add_block(struct acpi_table_iort *iort, struct acpi_iort_node *iort_node, IOVIRT_INFO_TABLE *iovirt_table, IOVIRT_BLOCK **block)
{
    uint32_t offset, *count, i;
    IOVIRT_BLOCK *next_block;
    NODE_DATA_MAP *data_map = &((*block)->data_map[0]);
    NODE_DATA *data = &((*block)->data);
    void *node_data = &(iort_node->node_data[0]);

    /* Populate the fields independent of node type */
    (*block)->type = iort_node->type;
    (*block)->num_data_map = iort_node->mapping_count;
    /* Populate fields dependent on node type */
    switch(iort_node->type)
    {
        case ACPI_IORT_NODE_ITS_GROUP:
            (*data).its_count = ((struct acpi_iort_its_group*)node_data)->its_count;
            /* ITS Group does not have ID mappings, but variable length array of identifiers */
            /* Populate the array here itself, and we are done with ITS group node */
            memcpy(&(*data_map).id[0], &((struct acpi_iort_its_group*)node_data)->identifiers[0], sizeof(uint32_t) * (*data).its_count);
            /* Override the num_data_map value. For every 4 ITS identifiers, */
            /* we have one data map */
            (*block)->num_data_map = ((*data).its_count + 3)/4;
            count = &iovirt_table->num_its_groups;
            break;
        case ACPI_IORT_NODE_NAMED_COMPONENT:
            strncpy((char*)(*data).name, (char*)((struct acpi_iort_named_component*)node_data)->device_name, 16);
            count = &iovirt_table->num_named_components;
            break;
        case ACPI_IORT_NODE_PCI_ROOT_COMPLEX:
            (*data).segment = ((struct acpi_iort_root_complex*)node_data)->pci_segment_number;
            memcpy(&(*data_map).id[0], &((struct acpi_iort_root_complex*)node_data)->memory_properties, sizeof(uint32_t) * 3);
            (*block)->num_data_map = 1;
            count = &iovirt_table->num_pci_rcs;
            break;
        case ACPI_IORT_NODE_SMMU:
            (*data).smmu.base = ((struct acpi_iort_smmu *)node_data)->base_address;
            (*data).smmu.arch_major_rev = 2;
            count = &iovirt_table->num_smmus;
            break;
        case ACPI_IORT_NODE_SMMU_V3:
            (*data).smmu.base = ((struct acpi_iort_smmu_v3 *)node_data)->base_address;
            (*data).smmu.arch_major_rev = 3;
            count = &iovirt_table->num_smmus;
            break;
        default:
            sbsa_print(AVS_PRINT_ERR, "Invalid IORT node type\n");
            return (uint32_t) -1;
    }

    /* Have we already added this block? */
    /* If so, return the block offset */
    offset = find_block(*block, iovirt_table);
    if(offset)
        return offset;

    (*block)->flags = 0;
    if(iort_node->type == ACPI_IORT_NODE_SMMU) {
        if(!smmu_ctx_int_distinct(ACPI_ADD_PTR(uint64_t, iort_node, ((struct acpi_iort_smmu *)node_data)->context_interrupt_offset),
                               ((struct acpi_iort_smmu *)node_data)->context_interrupt_count))
        (*block)->flags |= (1 << IOVIRT_FLAG_SMMU_CTX_INT_SHIFT);
    }

    /* Calculate the position where next block should be added */
    next_block = ACPI_ADD_PTR(IOVIRT_BLOCK, data_map, (*block)->num_data_map * sizeof(NODE_DATA_MAP));

    if((*block)->type != ACPI_IORT_NODE_ITS_GROUP)
    {
        struct acpi_iort_id_mapping *map = ACPI_ADD_PTR(struct acpi_iort_id_mapping, iort_node, iort_node->mapping_offset);
        /* For each id mapping copy the fields to corresponding data map fields */
        for(i = 0; i < (*block)->num_data_map; i++)
        {
            (*data_map).map.input_base = map->input_base;
            (*data_map).map.id_count = map->id_count;
            (*data_map).map.output_base = map->output_base;
            /* We don't know if the iort node referred to by map->output_*/
            /* reference is already added as a block. So try to add it and */
            /* store the returned offset in the relevant data map field. */
            /* We know this function will return offset of newly block or */
            /* already added block */
            offset = iovirt_add_block(iort,
                     ACPI_ADD_PTR(struct acpi_iort_node, iort, map->output_reference),
                     iovirt_table,
                     &next_block);
            (*data_map).map.output_ref = offset;
            data_map++;
            map++;
        }
    }
    /* So we successfully added a new block. Calculate its offset */
    offset = (uint8_t*)(*block) - (uint8_t*)iovirt_table;
    /* Inform the caller about the address at which next block must be added */
    *block = next_block;
    /* Increment the general and type specific block counters */
    iovirt_table->num_blocks++;
    *count =  *count + 1;
    return offset;
}

/**
  @brief  Parses ACPI IORT table and populates the local iovirt table
**/
void
pal_iovirt_create_info_table(IOVIRT_INFO_TABLE *iovirt_table)
{
    struct acpi_table_iort  *iort;
    struct acpi_iort_node   *iort_node, *iort_end;
    IOVIRT_BLOCK  *next_block;
    uint32_t i;

    if (iovirt_table == NULL)
        return;

    /* Initialize counters */
    iovirt_table->num_blocks = 0;
    iovirt_table->num_smmus = 0;
    iovirt_table->num_pci_rcs = 0;
    iovirt_table->num_named_components = 0;
    iovirt_table->num_its_groups = 0;

    iort = (struct acpi_table_iort *)pal_get_iort_ptr();

    if (iort == NULL) {
        return;
    }

    /* Point to the first Iovirt table block */
    next_block = &(iovirt_table->blocks[0]);

    /* Point to the first IORT node */
    iort_node = ACPI_ADD_PTR(struct acpi_iort_node, iort, iort->node_offset);
    iort_end = ACPI_ADD_PTR(struct acpi_iort_node, iort, iort->header.length);
    /* Create iovirt block for each IORT node*/
    for (i = 0; i < iort->node_count; i++) {
        if (iort_node >= iort_end) {
            sbsa_print(AVS_PRINT_ERR, "Bad IORT table \n");
            return;
        }
        iovirt_add_block(iort, iort_node, iovirt_table, &next_block);
        iort_node = ACPI_ADD_PTR(struct acpi_iort_node, iort_node, iort_node->length);
    }
    dump_iovirt_table(iovirt_table);
    check_mapping_overlap(iovirt_table);
}

/**
  @brief  Check if given SMMU node has unique context bank interrupt ids

  @param  smmu_block smmu IOVIRT block base address

  @return 0 if test fails, 1 if test passes
**/

uint32_t
pal_iovirt_check_unique_ctx_intid(uint64_t smmu_block)
{
    IOVIRT_BLOCK *block = (IOVIRT_BLOCK *)smmu_block;
    if(block->flags & (1 << IOVIRT_FLAG_SMMU_CTX_INT_SHIFT))
       return 0;
    return 1;

}

/**
  @brief  Check if given root complex node has unique requestor id to stream id mapping

  @param  rc_block root complex IOVIRT block base address

  @return 0 if test fails, 1 if test passes
**/

uint32_t
pal_iovirt_unique_rid_strid_map(uint64_t rc_block)
{
  IOVIRT_BLOCK *block = (IOVIRT_BLOCK *)rc_block;
  if(block->flags & (1 << IOVIRT_FLAG_STRID_OVERLAP_SHIFT))
    return 0;
  return 1;
}
