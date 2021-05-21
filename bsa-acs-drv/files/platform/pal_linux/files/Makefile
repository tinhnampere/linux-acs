## @file
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
 # Copyright (C) 2016-2018, 2021 Arm Limited
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

#since we have copied the files locally
ACS_DIR ?= .

PAL_SRC = $(ACS_DIR)/src

obj-m += bsa_acs_pal.o
bsa_acs_pal-objs += $(PAL_SRC)/pal_misc.o \
    $(PAL_SRC)/pal_pe.o   $(PAL_SRC)/pal_pcie.o   $(PAL_SRC)/pal_pcie_enumeration.o \
    $(PAL_SRC)/pal_smmu.o $(PAL_SRC)/pal_iovirt.o $(PAL_SRC)/pal_peripherals.o \
    $(PAL_SRC)/pal_dma.o  $(PAL_SRC)/pal_acpi.o $(PAL_SRC)/pal_gic.o $(PAL_SRC)/pal_exerciser.o \
    $(PAL_SRC)/pal_dt.o

ccflags-y=-I$(PWD)/$(ACS_DIR)/val/include -I$(PWD)/$(ACS_DIR)/include -I$(PWD) -I$(PWD)/../../  -I$(PWD)/../../../  -DTARGET_LINUX -Wall -Werror

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
