shell_folder=$(cd "$(dirname "$0")" || exit; pwd)
final="${shell_folder}"/../result/psort.c
makefile="${shell_folder}"/../result/makefile

if [ ! -d "$shell_folder"/../result ]; then
   echo '\033[32mCreating result...\033[0m'
   mkdir "$shell_folder"/../result
fi

echo '\033[32mGenerating psort.c...\033[0m'
sed '1d;2d;3d;$d' "${shell_folder}/../libpsort.h" > "${final}"
echo "\n" >> "${final}"
sed '1d;2d' "${shell_folder}/../libpsort.c" >> "${final}"
echo "\n" >> "${final}"
sed '1d;2d' "${shell_folder}/../main.c" >> "${final}"

echo '\033[32mGenerating makefile...\033[0m'
echo '
objs = psort.o
\n
psort: $(objs)\n
	gcc -o psort $(objs)\n
\n
psort.o: psort.c\n
	gcc -c psort.c\n
\n
clean: \n
	rm psort $(objs)\n
' > "${makefile}"
