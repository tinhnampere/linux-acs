
SUMMARY = "Compiles sbsa pal"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " file://Makefile \
            file://include/  \
            file://src/  \
            file://val/include/pal_interface.h \
	    file://COPYING \
            "
S = "${WORKDIR}"


do_install_append () {
        cp "${S}"/sbsa_acs_pal.o ${DEPLOY_DIR_IMAGE}/
}

