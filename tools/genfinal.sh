shell_folder=$(cd "$(dirname "$0")" || exit; pwd)
final="${shell_folder}"/../result/psort.c
filesize=${shell_folder}/../result/filesize.c
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

psort: $(objs)
	gcc -o psort $(objs)

psort.o: psort.c
	gcc -c psort.c

clean: 
	rm psort $(objs)
' > "${makefile}"


echo '
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


off_t get_file_size(const char *filename)  
{  
    struct stat buf;  
    if(stat(filename, &buf)<0)  
    {
        printf("cannot open file\\n");
        return 0;  
    } 
    printf("file: %s, st_size: %ld\\n", filename, (off_t)buf.st_size);
    return (off_t)buf.st_size;  
}

int main(int argc, char* argv[]){
    get_file_size(argv[1]);
    return 0;
}
' > "${filesize}"