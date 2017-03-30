SUMMARY = "Example of how to build an external Linux kernel module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://sbsa_acs_drv.c \
           file://sbsa_acs_drv.h \
           file://val/include/val_interface.h \
           file://val/include/pal_interface.h \
           file://val/include/sbsa_avs_cfg.h \
           file://val/include/sbsa_avs_common.h \
           file://val/include/sbsa_avs_pcie.h \
           file://val/include/sbsa_avs_val.h \
           file://val/include/sbsa_avs_dma.h \
           file://val/include/sbsa_avs_pe.h \
           file://val/include/sbsa_std_smc.h \
           file://val/include/sbsa_avs_peripherals.h \
           file://val/include/sbsa_avs_iovirt.h \
           file://val/include/sbsa_avs_smmu.h \
           file://val/include/sbsa_avs_memory.h \
           file://platform/pal_linux/include/pal_linux.h \
           file://platform/pal_linux/include/sbsa_pcie_enum.h \
           file://platform/pal_linux/src/pal_misc.c \
           file://platform/pal_linux/src/pal_pe.c \
           file://platform/pal_linux/src/pal_smmu.c \
           file://platform/pal_linux/src/pal_pcie.c \
           file://platform/pal_linux/src/pal_pcie_enumeration.c \
           file://platform/pal_linux/src/pal_iovirt.c \
           file://platform/pal_linux/src/pal_dma.c \
           file://platform/pal_linux/src/pal_acpi.c \
           file://platform/pal_linux/src/pal_peripherals.c \
           file://val/src/avs_status.c \
           file://val/src/avs_dma.c \
           file://val/src/avs_test_infra.c \
           file://val/src/avs_memory.c \
           file://val/src/avs_peripherals.c \
           file://val/src/avs_smmu.c \
           file://val/src/avs_iovirt.c \
           file://val/src/avs_pcie.c \
           file://val/src/avs_pe_infra.c \
           file://test_pool/pcie/test_p001.c \
           file://test_pool/pcie/test_p002.c \
           file://test_pool/pcie/test_p003.c \
           file://test_pool/pcie/test_p004.c \
           file://test_pool/pcie/test_p005.c \
           file://test_pool/pcie/test_p006.c \
           file://test_pool/pcie/test_p007.c \
           file://COPYING \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
