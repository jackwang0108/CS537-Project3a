#include "psort.h"


int main(int argc, char* argv[]){

    char *buffer = (char *) malloc(sizeof(char) * 10);
    for (int i = 1; i < argc; i++){
        int len = read_records(argv[i], &buffer);
        printf("%s -> %d\n", argv[i], len);
        printf("%s", byte2char(buffer, len));
    }

    return 0;
}