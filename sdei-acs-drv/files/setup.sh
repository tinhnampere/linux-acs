#!/bin/bash

echo "=================================================================="
echo "This script will copy the necessary files for Kernel module build "
echo "to the present directory"
echo "=================================================================="

if [ $# -ne 1 ];
then
    echo "Give SDEI_ACS_PATH as the arguments to the script"
    return 0
fi

export SDEI_ACS_PATH=$1
echo -e "SDEI_ACS_PATH is set to -> \e[93m $SDEI_ACS_PATH\e[0m"

cp -r $SDEI_ACS_PATH/val .
cp -r $SDEI_ACS_PATH/platform/pal_linux .
cp -r $SDEI_ACS_PATH/test_pool .
