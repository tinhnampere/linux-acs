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
echo "This script will copy the necessary files for Kernel module build "
echo "to the present directory"
echo "=================================================================="

if [ $# -ne 1 ];
then
    echo "Give ACS_PATH as the arguments to the script"
    return 0
fi

export ACS_PATH=$1
echo -e "ACS_PATH is set to -> \e[93m $ACS_PATH\e[0m"

cp -r $ACS_PATH/val .
cp -r $ACS_PATH/test .
cp -r $ACS_PATH/platform/common .
