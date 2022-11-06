# Notes: 过时的测试代码，Piazza上更新了官方的测试代码
# if [ $# -eq 1 ]; then 
#     dd if=/dev/random of=test.bin bs="$1" count=1 iflag=fullblock
# else
#     dd if=/dev/random of=test.bin bs=1000 count=1 iflag=fullblock
# fi

shell=$(echo $SHELL)
shell_folder=$(cd "$(dirname "$0")" || exit; pwd)

$shell "$shell_folder"/autobuild.sh
if [ $# -eq 1 ]; then
    echo "Writing $1 records to test.bin..."
    "$shell_folder"/../bin/gentest "$1" "$shell_folder"/../test.bin
else
    echo "Writing 10 records to test.bin..."
    "$shell_folder"/../bin/gentest 10 "$shell_folder"/test.bin
fi