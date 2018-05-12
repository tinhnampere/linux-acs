## @file
 # SDEI ACS Kernel module Makefile
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
 # Copyright (C) 2018 Arm Limited
 #
 # Author: Mahesh Bireddy <mahesh.reddybireddy@arm.com>
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

#since we have copied the files locally
ACS_DIR ?= ./

obj-m += sdei_acs.o
sdei_acs-objs += sdei_acs_drv.o \
                 sdei_acs_pal.o \
                 sdei_acs_val.o \
		 sdei_acs_test.o

ccflags-y=-I$(PWD)/$(ACS_DIR)/val/include  -I$(PWD)/$(ACS_DIR)/ -DTARGET_LINUX -Wall -Werror

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
