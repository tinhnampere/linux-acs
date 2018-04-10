
SUMMARY = "Example of how to build an external Linux kernel module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " file://Makefile \
            file://sdei_acs_drv.c \
            file://sdei_acs_drv.h \
            file://val/Makefile \
            file://platform/pal_linux/Makefile \
            file://val/include/pal_interface.h \
            file://val/include/val_interface.h \
            file://val/include/val_pe.h \
            file://val/include/val_sdei_interface.h \
            file://val/include/val_specification.h \
            file://val/include/val_test_infra.h \
            file://val/include/val_timer.h \
            file://platform/pal_linux/include/pal_linux.h \
            file://platform/pal_linux/include/pal_sdei_interface.h \
            file://platform/pal_linux/src/pal_acpi.c \
            file://platform/pal_linux/src/pal_gic.c \
            file://platform/pal_linux/src/pal_misc.c \
	        file://platform/pal_linux/src/pal_pe.c \
            file://platform/pal_linux/src/pal_psci.c \
            file://platform/pal_linux/src/pal_sdei_interface.c \
            file://platform/pal_linux/src/pal_wd_timer.c \
            file://val/src/val_misc.c \
            file://val/src/val_psci.c \
            file://val/src/val_test_infra.c \
            file://val/src/val_timer_support.c \
            file://val/src/val_gic.c \
            file://val/src/val_pe.c \
            file://val/src/val_sdei_interface.c \
            file://val/src/val_timer.c \
            file://val/src/val_wd_timer.c \
            file://val/src/AArch64/ArchTimerSupport.S \
            file://val/src/AArch64/event_handler.S \
            file://val/src/AArch64/PeRegSysSupport.S \
            file://test_pool/test_001.c \
            file://test_pool/test_002.c \
            file://test_pool/test_003.c \
            file://test_pool/test_004.c \
            file://test_pool/test_005.c \
            file://test_pool/test_006.c \
            file://test_pool/test_007.c \
            file://test_pool/test_008.c \
            file://test_pool/test_009.c \
            file://test_pool/test_010.c \
            file://test_pool/test_011.c \
            file://test_pool/test_012.c \
            file://test_pool/test_013.c \
            file://test_pool/test_014.c \
            file://test_pool/test_015.c \
            file://test_pool/test_016.c \
            file://test_pool/test_017.c \
            file://test_pool/test_018.c \
            file://test_pool/test_019.c \
            file://test_pool/test_020.c \
            file://test_pool/test_021.c \
            file://test_pool/test_022.c \
            file://test_pool/test_023.c \
            file://test_pool/test_024.c \
            file://test_pool/test_025.c \
            file://test_pool/test_026.c \
            file://test_pool/test_027.c \
            file://test_pool/test_028.c \
            file://test_pool/test_029.c \
            file://test_pool/test_030.c \
            file://test_pool/test_031.c \
            file://test_pool/test_032.c \
            file://test_pool/test_033.c \
            file://test_pool/test_034.c \
            file://test_pool/test_035.c \
            file://test_pool/test_036.c \
            file://test_pool/test_037.c \
            file://test_pool/test_038.c \
            file://test_pool/test_039.c \
            file://test_pool/test_040.c \
            file://test_pool/test_041.c \
            file://test_pool/test_042.c \
            file://test_pool/test_043.c \
            file://test_pool/test_044.c \
            file://test_pool/test_045.c \
            file://test_pool/test_046.c \
            file://test_pool/test_047.c \
            file://test_pool/test_048.c \
            file://test_pool/test_049.c \
            file://COPYING \
            "
S = "${WORKDIR}"

