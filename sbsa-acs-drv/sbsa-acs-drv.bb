SUMMARY = "Compiles driver. This depends on pal and val compilation"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

do_configure[depends] += "sbsa-acs-pal:do_install \
                          sbsa-acs-val:do_install \
                          sbsa-acs-test:do_install  "

SRC_URI = " file://Makefile \
            file://sbsa_acs_drv.c \
            file://sbsa_acs_drv.h \
            file://val/include \
            file://COPYING \
            "
S = "${WORKDIR}"

do_configure_append () {
  cp ${DEPLOY_DIR_IMAGE}/sbsa_acs_pal.o ${S}
  cp ${DEPLOY_DIR_IMAGE}/sbsa_acs_val.o ${S}
  cp ${DEPLOY_DIR_IMAGE}/sbsa_acs_test.o ${S}
}

do_install_append () {
  rm ${DEPLOY_DIR_IMAGE}/sbsa_acs_pal.o
  rm ${DEPLOY_DIR_IMAGE}/sbsa_acs_val.o
  rm ${DEPLOY_DIR_IMAGE}/sbsa_acs_test.o
}
# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
