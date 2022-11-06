shell_folder=$(cd "$(dirname "$0")" || exit; pwd)

if [ ! -d "$shell_folder"/../build ]; then
   echo '\033[32mCreating build...\033[0m'
   mkdir "$shell_folder"/../build
fi
cd "$shell_folder"/../build || exit

echo '\033[32mBuilding...\033[0m'
cmake ..
make
echo '\033[32mFinished building, everthing up-to-date...\033[0m'