shell=$(echo $SHELL)
shell_folder=$(cd "$(dirname "$0")" || exit; pwd)

$shell "$shell_folder"/autobuild.sh
echo '\033[32mTesting...\033[0m'
"$shell_folder"/../bin/psort "$shell_folder"/../test.bin "$shell_folder"/../sorted.bin
echo '\033[32mDone! Sorted record written to sorted.bin\033[0m'
echo '\033[32mRun printkey.sh to check keys of sorted.bin\033[0m'