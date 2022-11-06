shell_folder=$(cd "$(dirname "$0")" || exit; pwd)
final="${shell_folder}"/psort.c

echo '\033[32mGenerating psort.c...\033[0m'
sed '1d;2d;3d;$d' "${shell_folder}/libpsort.h" > "${final}"
echo "\n" >> "${final}"
sed '1d;2d' "${shell_folder}/libpsort.c" >> "${final}"
echo "\n" >> "${final}"
sed '1d;2d' "${shell_folder}/main.c" >> "${final}"
