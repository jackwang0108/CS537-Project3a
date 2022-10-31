#include "psort.h"


int main(int argc, char* argv[]){

    byteStream buffer = (byteStream) malloc(sizeof(char) * 10);
    for (int i = 1; i < argc; i++){
        int len = read_records(argv[i], &buffer);
        printf("%s -> %d\n", argv[i], len);

        record_t *records;
        int num = parse_records(buffer, &records, len);
        printf("%s\n", byte2char(*records[0], RECORD_SIZE));

        compare(records[0], records[1]);
    }

    return 0;
}