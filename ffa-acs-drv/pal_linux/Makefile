## @file
 # FFA ACS Kernel module Makefile
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
 # Copyright (C) 2021 Arm Limited
 #
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

# ACS Make commandline arguments
VERBOSITY ?= 3
PLATFORM_NS_HYPERVISOR_PRESENT ?=  0
PLATFORM_SPMC_EL ?= 2
PLATFORM_SP_EL   ?= 1
SUITE            ?= all
PLATFORM_FFA_V_1_0 ?= 0
PLATFORM_FFA_V_1_1 ?= 0
PLATFORM_FFA_V_ALL ?= 1

ACS_MACROS += -DVM1_COMPILE -DTARGET_LINUX
ACS_MACROS += -DVERBOSITY=$(VERBOSITY)
ACS_MACROS += -DPLATFORM_NS_HYPERVISOR_PRESENT=$(PLATFORM_NS_HYPERVISOR_PRESENT)
ACS_MACROS += -DPLATFORM_SPMC_EL=$(PLATFORM_SPMC_EL)
ACS_MACROS += -DPLATFORM_SP_EL=$(PLATFORM_SP_EL)
ACS_MACROS += -DSUITE=$(SUITE)
ACS_MACROS += -DPLATFORM_FFA_V_1_0=$(PLATFORM_FFA_V_1_0)
ACS_MACROS += -DPLATFORM_FFA_V_1_1=$(PLATFORM_FFA_V_1_1)
ACS_MACROS += -DPLATFORM_FFA_V_ALL=$(PLATFORM_FFA_V_ALL)

#since we have copied the files locally
ACS_DIR ?= .
PAL_SRC = $(ACS_DIR)/src

ACS_INCLUDE = -I$(PWD)/$(ACS_DIR)/inc -I$(PWD)/$(ACS_DIR)/../common/inc
obj-m += ffa_acs_pal.o

ffa_acs_pal-objs += $(PAL_SRC)/pal_driver.o \
                    $(PAL_SRC)/pal_mmio.o \
                    $(PAL_SRC)/pal_irq.o \
                    $(PAL_SRC)/pal_smmuv3_testengine.o \
                    $(PAL_SRC)/pal_vcpu_setup.o \
                    $(PAL_SRC)/pal_misc.o

ccflags-y=$(ACS_MACROS) $(ACS_INCLUDE) -Wall -Werror

all:
ifeq ($(KERNEL_SRC),)
	echo "  KERNEL_SRC variable should be set to kernel path "
	exit 1
else
	echo "Kernel source is set to $(KERNEL_SRC)"
endif

	make -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
	find $(ACS_DIR) -type f -name "*.o" -delete
	find $(ACS_DIR) -type f -name "*.o.cmd" -delete

