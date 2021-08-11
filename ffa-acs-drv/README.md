
# FF-A ACS - Linux Kernel Module

This document provides information on executing [ff-a-acs tests](https://github.com/ARM-software/ff-a-acs.git) from the Linux application. This needs Linux kernel module files to load FF-A ACS Test VM1 endpoint code at kernel level and run as part of Linux kernel. These files are hosted under ffa-acs-drv directory. The procedure to build and run FF-A tests with Linux is described here.

## This release
- Release Version - v0.8
- Maintained by [ff-a-acs](https://github.com/ARM-software/ff-a-acs.git) manitainers

## Prerequisites
- Linux kernel source version 5.0 or 5.12
- Linaro GCC tool chain 5.3 or above
- Build environment for AArch64 Linux kernel

## Building the kernel module
Follow the below steps to create kernel module for VM1 endpoint code:

* Checkout the sources:
1. mkdir <local_dir> ; cd <local_dir>
1. git clone https://git.gitlab.arm.com/linux-arm/linux-acs.git
2. git clone https://github.com/ARM-software/ff-a-acs.git

* Porting steps for Linux kernel
1. Apply the <local_dir>/linux-acs/ffa-acs-drv/kernel/0001-FFA-ACS-Linux-v5.0.patch to your kernel-v5.0 source tree.
2. Apply the <local_dir>/linux-acs/ffa-acs-drv/kernel/0001-FFA-ACS-Linux-v5.12.patch to your kernel-v5.12 source tree.
3. Build the kernel.

* Build steps for FFA ACS kernel module
1. cd <local_dir>/linux-acs/ffa-acs-drv
2. Set CROSS_COMPILE to the ARM64 toolchain path. You may want to set additional env flags mentioned in **Option Information** to change the default values of the ACS Make variables.
3. Set KERNEL_SRC to the <path_to_Linux_kernel>
4. ./setup.sh <path_to_ffa-a-acs>
5. ./linux_ffa_acs.sh

* ffa-acs.ko file is generated for VM1 endpoint code.<br />

**Options information**:<br />
   1. SUITE=<suite_name> is the sub test suite name specified in **test/** directory. The default value is -DSUITE=all<br />
   2. VERBOSE=<verbose_level>. Print verbosity level. Supported print levels are 1(INFO & above), 2(DEBUG & above), 3(TEST & above), 4(WARN & ERROR) and 5(ERROR). Default value is 3.<br />
   3. PLATFORM_SPMC_EL=<el_num>: EL number where the target SPMC component runs. Supported values are 1 and 2. The default value is 2.<br />
   4. PLATFORM_SP_EL=<el_num>: EL number where the test secure endpoints are expected to run. Supported values are 0 and 1. The default value is 1.<br />
   5. PLATFORM_NS_HYPERVISOR_PRESENT=<0|1>: Does the system support the non-secure hypervisor component? 1 for yes, 0 for no. The default vaule is 0. System is expected to intergrate and load all the three of nonsecure test endpoints if the value is set to 1. Otherwise needs to use single non-secure test endpoint(vm1) which would act as NS OS kernel.<br />


## Build the Linux Application
Perform below steps to generate ffa_acs executable file:<br />
1. cd <local_dir>/ffa-a-acs/linux_app/
2. Export CROSS_COMPILE with the ARM64 toolchain path.
3. make


## Loading the kernel module
Before the FF-A ACS Linux application can run, load the FFA ACS kernel module using the
insmod command.<br />
    ```
    shell> insmod ffa-acs.ko
    ```
## Run the Linux application

Run the FF-A ACS Linux application as below:<br />
```
    shell> ./ffa_acs
```
