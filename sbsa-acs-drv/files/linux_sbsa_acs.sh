#!/bin/bash

echo "=================================================================="
echo "This script will compile the Linux SBSA Kernel module  "
echo "=================================================================="

SBSA_ACS_PATH=`pwd`
if [ -z $KERNEL_SRC ]
then
    echo "Error: set KERNEL_SRC path"
    exit 0
fi

if [ -z $CROSS_COMPILE ]
then
    echo "Error: set CROSS_COMPILE path"
    exit 0
fi

cd $SBSA_ACS_PATH/pal_linux/files
make
cp sbsa_acs_pal.o $SBSA_ACS_PATH/

cd $SBSA_ACS_PATH/val/
make
cp sbsa_acs_val.o $SBSA_ACS_PATH/

cd $SBSA_ACS_PATH/test_pool/
make
cp sbsa_acs_test.o $SBSA_ACS_PATH/

cd $SBSA_ACS_PATH/
make
