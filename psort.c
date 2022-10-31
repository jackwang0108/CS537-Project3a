#include "psort.h"


// Notes: 
//    目前的打算是分块读取文件，然后每个块内使用快排/Alpha排序……排序方法
//    然后每个块使用一个单独的线程来排序
//    每个块排序完毕之后再使用归并排序或者其他递归排序方法进行排序
// Attention:
//    估计测试文件会很大，不然没有办法有效地测试性能
//    所以我严重怀疑等截止日期快到的时候实验机器会变得极其卡顿
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