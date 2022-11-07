#include "libpsort.h"


int main(int argc, char *argv[]){
    byteStream buffer;
    int byte = read_records(argv[1], &buffer, 0, -1);
    printf("PrintKey file: %s\n", argv[1]);
    printf("%s -> %d bytes\n", argv[1], byte);
    record_t *records;
    int num = parse_records(buffer, &records, byte);
    printf("%s -> %d records\n", argv[1], num);
    delim;
    printKeys(records, num);
    return 0;
}