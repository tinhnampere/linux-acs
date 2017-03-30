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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/transport_class.h>
#include <linux/libata.h>
#include <asm/unaligned.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_transport.h>

#include <asm/io.h>

#include "include/pal_linux.h"
#include "include/sbsa_pcie_enum.h"
#include <linux/dma-mapping.h>
#include <linux/sbsa-iommu.h>

int
sbsa_scsi_sata_get_dma_addr(struct ata_port *ap, dma_addr_t *dma_addr, unsigned int *dma_len);


unsigned long long int
pal_dma_mem_alloc(void **buffer, unsigned int length, void *port, unsigned int flags)
{
        dma_addr_t mem_dma;

        if (flags == DMA_COHERENT) {
                *buffer = dmam_alloc_coherent(((struct ata_port *)port)->dev, length, &mem_dma, GFP_KERNEL);
                if (!(*buffer)) {
                        printk(KERN_ERR "SBSA-DRV - Alloc failure %s \n", __func__);
                        return -ENOMEM;
                }
        } else {
                *buffer = kmalloc(length, GFP_KERNEL);
                mem_dma = dma_map_single(((struct ata_port *)port)->dev, *buffer, length, DMA_BIDIRECTIONAL);
                if (dma_mapping_error(((struct ata_port *)port)->dev, mem_dma)) {
                        pr_err("SBSA_DRV : DMA Map single page failed.\n");
                        kfree(*buffer);
                        return -1;
                }
        }
        memset(*buffer, 0, length);

        //printk("dma buffer is %llx %llx \n", *buffer, mem_dma);

        return mem_dma;
}

void
pal_dma_scsi_get_dma_addr(void *port, void *dma_addr, unsigned int *dma_len)
{

	sbsa_scsi_sata_get_dma_addr(port, dma_addr, dma_len);
}

void
pal_dma_mem_free(void *buffer, dma_addr_t mem_dma, unsigned int length, void **port, unsigned int flags)
{

       dmam_free_coherent(((struct ata_port *)port)->dev, length, buffer, mem_dma);

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

	//printk("Look at all SCSI hosts in the system \n");
	do {
		shost = scsi_host_lookup(i++);
		if (shost) {
			//printk("Found a scsi host: ");
			sdev = NULL;
			ap = ata_shost_to_port(shost);
			if (ap == NULL)
				continue; //Not a ATA port
			do {
				/* get the device connected to this host */
				sdev = __scsi_iterate_devices(shost, sdev);
				if (sdev) {
					//printk(" found device connected to this host \n");
					dma_info_table->info[j].host   = shost;
					dma_info_table->info[j].port   = ap;
					dma_info_table->info[j].target = sdev;
					dma_info_table->info[j].flags  = sbsa_dev_get_dma_attr(shost->dma_dev);

					/* if we did not get coherence attribute from ACPI/PCI, get it from FDT */
					if (dma_info_table->info[j].flags == 0) {
						dma_info_table->info[j].flags = shost->dma_dev->archdata.dma_coherent;
					}
					if (ap->dev->bus->iommu_ops)
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

        memset(cmd, 0, 15);

        cmd[0] = WRITE_10;
        cmd[8] = 1;  //one block only for now

        ret = scsi_execute_req(sdev, cmd, DMA_TO_DEVICE, dma_source_buf, length,
                               NULL, timeout, 1, NULL);

        return 0;
}


