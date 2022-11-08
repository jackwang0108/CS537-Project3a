shell=$(echo $SHELL)
shell_folder=$(cd "$(dirname "$0")" || exit; pwd)

echo "\033[32mTesting...\033[0m"
$shell "$shell_folder"/autobuild.sh

echo "\033[32mGenerating submission...\033[0m"
$shell "$shell_folder"/genfinal.sh

echo "\033[32mOfficial testing...\033[0m"
cd ${shell_folder}/../result
make
$shell ~cs537-1/tests/p3a/test-psort.sh -c