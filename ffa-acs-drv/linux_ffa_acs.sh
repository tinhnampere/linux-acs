#!/bin/bash
##
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
##

echo "=================================================================="
echo "This script will compile the Linux FFA ACS Kernel module  "
echo "=================================================================="

FFA_ACS_PATH=`pwd`
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

cd $FFA_ACS_PATH/pal_linux
make clean
make
cp ffa_acs_pal.o $FFA_ACS_PATH/

cd $FFA_ACS_PATH/val/
make clean
make
cp ffa_acs_val.o $FFA_ACS_PATH/

cd $FFA_ACS_PATH/test/
make clean
make
cp ffa_acs_test.o $FFA_ACS_PATH/

cd $FFA_ACS_PATH/
make
