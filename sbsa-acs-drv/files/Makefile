## @file
 # SBSA ACS Kernel module Makefile
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License version 2 as
 # published by the Free Software Foundation.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
 #
 # Copyright (C) 2017 ARM Limited
 #
 # Author: Prasanth Pulla <prasanth.pulla@arm.com>
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

#since we have copied the files locally
ACS_DIR ?= ./

PAL_SRC = $(ACS_DIR)/platform/pal_linux/src
VAL_SRC = $(ACS_DIR)/val/src
TEST_POOL = $(ACS_DIR)/test_pool

obj-m += sbsa_acs.o
sbsa_acs-objs += sbsa_acs_drv.o $(PAL_SRC)/pal_misc.o \
    $(PAL_SRC)/pal_pe.o   $(PAL_SRC)/pal_pcie.o   $(PAL_SRC)/pal_pcie_enumeration.o \
    $(PAL_SRC)/pal_smmu.o $(PAL_SRC)/pal_iovirt.o $(PAL_SRC)/pal_peripherals.o \
    $(PAL_SRC)/pal_dma.o  $(PAL_SRC)/pal_acpi.o \
    $(VAL_SRC)/avs_status.o      $(VAL_SRC)/avs_memory.o \
    $(VAL_SRC)/avs_peripherals.o $(VAL_SRC)/avs_dma.o  $(VAL_SRC)/avs_smmu.o \
    $(VAL_SRC)/avs_test_infra.o  $(VAL_SRC)/avs_pcie.o $(VAL_SRC)/avs_pe_infra.o \
    $(VAL_SRC)/avs_iovirt.o \
    $(TEST_POOL)/pcie/test_p001.o $(TEST_POOL)/pcie/test_p002.o \
    $(TEST_POOL)/pcie/test_p003.o $(TEST_POOL)/pcie/test_p004.o \
    $(TEST_POOL)/pcie/test_p005.o $(TEST_POOL)/pcie/test_p006.o \
    $(TEST_POOL)/pcie/test_p007.o

ccflags-y=-I$(PWD)/$(ACS_DIR)/val/ -I$(PWD)/$(ACS_DIR)/platform/pal_linux/ -I$(PWD)/$(ACS_DIR)/ -DTARGET_LINUX

all:
ifeq ($(KERNEL_SRC),)
	echo "	KERNEL_SRC variable should be set to kernel path "
	exit 1
else
	echo "Kernel source is set to $(KERNEL_SRC)"
endif

	make -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
