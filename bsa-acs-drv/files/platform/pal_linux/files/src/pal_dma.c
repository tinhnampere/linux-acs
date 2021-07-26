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
 * Copyright (C) 2016-2018,2021 Arm Limited
 *
 * Author: Prasanth Pulla <prasanth.pulla@arm.com>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/completion.h>
#include <linux/transport_class.h>
#include <linux/libata.h>
#include <asm/unaligned.h>
#include <asm/pgtable.h>
#include <asm/sysreg.h>
#include <asm/pgtable-types.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport.h>

#include <asm/io.h>

#include "include/pal_linux.h"
#include "include/bsa_pcie_enum.h"
#include <linux/dma-mapping.h>
#include <linux/bsa-iommu.h>

int
bsa_scsi_sata_get_dma_addr(struct ata_port *ap, dma_addr_t *dma_addr, unsigned int *dma_len);


unsigned long long int
pal_dma_mem_alloc(void **buffer, unsigned int length, void *port, unsigned int flags)
{
        dma_addr_t mem_dma;

        if (flags == DMA_COHERENT) {
                *buffer = dmam_alloc_coherent(((struct ata_port *)port)->dev, length, &mem_dma, GFP_KERNEL);
                if (!(*buffer)) {
                        pr_err ("BSA-DRV - Alloc failure %s \n", __func__);
                        return -ENOMEM;
                }
        } else {
                *buffer = kmalloc(length, GFP_KERNEL);
                mem_dma = dma_map_single(((struct ata_port *)port)->dev, *buffer, length, DMA_BIDIRECTIONAL);
                if (dma_mapping_error(((struct ata_port *)port)->dev, mem_dma)) {
                        pr_err("BSA_DRV : DMA Map single page failed.\n");
                        kfree(*buffer);
                        return -1;
                }
        }
        memset(*buffer, 0, length);

        return mem_dma;
}

void
pal_dma_scsi_get_dma_addr(void *port, void *dma_addr, unsigned int *dma_len)
{

	bsa_scsi_sata_get_dma_addr(port, dma_addr, dma_len);
}

void
pal_dma_mem_free(void *buffer, addr_t mem_dma, unsigned int length, void *port, unsigned int flags)
{

    if (flags == DMA_COHERENT) {
        dmam_free_coherent(((struct ata_port *)port)->dev, length, buffer, mem_dma);
    } else {
        dma_unmap_single(((struct ata_port *)port)->dev, mem_dma, length, DMA_BIDIRECTIONAL);
        kfree(buffer);
    }

}


void
pal_dma_poll_for_completion(unsigned int timeout)
{

        /* scsi execute takes care of this. */
        return;

}

void
pal_dma_create_info_table(DMA_INFO_TABLE *dma_info_table)
{

	struct Scsi_Host   *shost;
	struct ata_port    *ap;
	struct scsi_device *sdev = NULL;
	unsigned int i = 0, j = 0;

	dma_info_table->num_dma_ctrls = 0;

	do {
		shost = scsi_host_lookup(i++);
		if (shost) {
			sdev = NULL;
			ap = ata_shost_to_port(shost);
			if ((ap == NULL) || (ap->dev == NULL))
				continue; //Not a ATA port
			do {
				/* get the device connected to this host */
				sdev = __scsi_iterate_devices(shost, sdev);
				if (sdev) {
					dma_info_table->info[j].host   = shost;
					dma_info_table->info[j].port   = ap;
					dma_info_table->info[j].target = sdev;
					dma_info_table->info[j].flags  = bsa_dev_get_dma_attr(shost->dma_dev);

					/* if we did not get coherence attribute from ACPI/PCI, get it from FDT */
					if (dma_info_table->info[j].flags == 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0)
						dma_info_table->info[j].flags = shost->dma_dev->dma_coherent;
#else
						dma_info_table->info[j].flags = shost->dma_dev->archdata.dma_coherent;
#endif
					}
					if (pal_smmu_check_dev_attach(ap->dev))
						dma_info_table->info[j].flags |= IOMMU_ATTACHED;
					dma_info_table->info[j++].type = sdev->type;
					dma_info_table->num_dma_ctrls++;
				}
			} while(sdev);
			scsi_host_put(shost);
		}
	} while(shost);

}

unsigned int
pal_dma_start_from_device(void *dma_target_buf, unsigned int length,
                          void *host, void *dev)
{
        unsigned char scsi_cmd[16];
        int result;
        struct scsi_sense_hdr sshdr;
        struct scsi_device *sdev = (struct scsi_device *)dev;


        memset(&scsi_cmd[0], 0, 15);
        scsi_cmd[0] = READ_10;

        /*
         * bytes 7 - 8: length of the command.
         */
        // put_unaligned_le16(length, &scsi_cmd[7]);
        scsi_cmd[8] = 1; //Hard-coded to 1 block for now

        scsi_cmd[10] = 0;       /* reserved */
        scsi_cmd[11] = 0;       /* control */

        result = scsi_execute_req(sdev, scsi_cmd, DMA_FROM_DEVICE,
                                          dma_target_buf, length, &sshdr,
                                          10000, 3, NULL);

        return result;
}

unsigned int
pal_dma_start_to_device(void *dma_source_buf, unsigned int length,
                         void *host, void *target, unsigned int timeout)
{

        unsigned char cmd[10];
        int ret;
        struct scsi_device *sdev = (struct scsi_device *)target;

        memset(cmd, 0, 10);

        cmd[0] = WRITE_10;
        cmd[8] = 1;  //one block only for now

        ret = scsi_execute_req(sdev, cmd, DMA_TO_DEVICE, dma_source_buf, length,
                               NULL, timeout, 1, NULL);

        return 0;
}


static int
is_pte(uint64_t val)
{
    return (val & 0x2);
}

/* Decode memory attribute and shareabilty from page table descriptor val*/
static void
decode_mem_attr_sh(uint64_t val, uint32_t *attr, uint32_t *sh)
{
    uint64_t mair = read_sysreg(mair_el1);
    uint32_t attrindx = (val & PTE_ATTRINDX_MASK) >> 2;
    *attr = (mair >> (attrindx * 8)) & 0xff;
    *sh = (val & PTE_SHARED) >> 8;
    pr_info("In decode_mem_attr_sh with attr=%x, sh=%x\n", *attr, *sh);
}

int
pal_dma_mem_get_attrs(void *buf, uint32_t *attr, uint32_t *sh)
{
    pud_t *pud = NULL;
    pmd_t *pmd;
    pte_t *pte;
    pgd_t *pgd, *swapper_pgd;
    struct page *pg = phys_to_page(read_sysreg(ttbr1_el1));

    swapper_pgd = (pgd_t*) kmap(pg);
    if(!swapper_pgd)
        return -1;

    pgd = pgd_offset_k((uint64_t)buf);
    if(!pgd)
        return -1;
    kunmap(pg);

    pud = pud_offset((p4d_t *) pgd, (uint64_t)buf);
    if(!pud)
        return -1;

    if(is_pte(pud_val(*pud))) {
         decode_mem_attr_sh(pud_val(*pud), attr, sh);
         return 0;
    }

    pmd = pmd_offset(pud, (uint64_t)buf);
    if(!pmd)
        return -1;
    if(is_pte(pmd_val(*pmd))) {
         decode_mem_attr_sh(pmd_val(*pmd), attr, sh);
         return 0;
    }

    pte = pte_offset_kernel(pmd, (uint64_t)buf);
    if(!pte)
        return -1;

    if(is_pte(pte_val(*pte))) {
        decode_mem_attr_sh(pte_val(*pte), attr, sh);
        return 0;
    }

    return -1;
}
