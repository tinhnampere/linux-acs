
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
cp -r $ACS_PATH/test_pool .
