
echo "==================================================================
echo "This script will copy the necessary files for Kernel module build "
echo "to the present directory"
echo "==================================================================

if [ $# -ne 1 ];
then
    echo "Give AVS_PATH as the arguments to the script"
    return 0
fi

export AVS_PATH=$1
echo -e "AVS_PATH is set to -> \e[93m $AVS_PATH\e[0m"

cp -r $AVS_PATH/val .
cp -r $AVS_PATH/test_pool .
