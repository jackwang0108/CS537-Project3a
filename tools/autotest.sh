shell=$(echo $SHELL)
shell_folder=$(cd "$(dirname "$0")" || exit; pwd)

$shell "$shell_folder"/../autobuild.sh
"$shell_folder"/../bin/psort "$shell_folder"/../test.bin