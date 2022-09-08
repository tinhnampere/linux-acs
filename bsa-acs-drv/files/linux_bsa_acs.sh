#!/bin/bash

echo "=================================================================="
echo "This script will compile the Linux BSA Kernel module  "
echo "=================================================================="

BSA_ACS_PATH=`pwd`
if [ -z $KERNEL_SRC ]
then
    echo "Error: set KERNEL_SRC path"
    exit 0
fi

if [ $(uname -m) != "aarch64" ] && [ -z $CROSS_COMPILE ]
then
    echo "Error: set CROSS_COMPILE path for cross compile"
    exit 0
fi

cd $BSA_ACS_PATH/platform/pal_linux/files
make
cp bsa_acs_pal.o $BSA_ACS_PATH/

cd $BSA_ACS_PATH/val/
make
cp bsa_acs_val.o $BSA_ACS_PATH/

cd $BSA_ACS_PATH/test_pool/
make
cp bsa_acs_test.o $BSA_ACS_PATH/

cd $BSA_ACS_PATH/
make
