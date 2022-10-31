#include "psort.h"


// Notes: 
//    目前的打算是分块读取文件，然后每个块内使用快排/Alpha排序……排序方法
//    然后每个块使用一个单独的线程来排序
//    每个块排序完毕之后再使用归并排序或者其他递归排序方法进行排序

// Attention:
//    估计测试文件会很大，不然没有办法有效地测试性能
//    所以我严重怀疑等截止日期快到的时候实验机器会变得极其卡顿

// Warning:
//    使用tmux和printKey在一起有BUG，输出不全而且可能会出错（record数量 > 1000时），初步猜测是tmux的字符缓冲区有问题，来不及刷新
int main(int argc, char* argv[]){

    byteStream buffer = (byteStream) malloc(sizeof(char) * 10);
    for (int i = 1; i < argc; i++){
        int len = read_records(argv[i], &buffer, 0, -1);
        printf("%s -> %d\n", argv[i], len);

        record_t *records;
        int num = parse_records(buffer, &records, len);
        printf("%s\n", byte2char(*records[0], BYTE_PER_RECORD));

        printf("%d\n", less_than(records[0], records[1]));

        // printKeys(records, num);

        clock_t start, end;
        double total;

        start = clock();
        // bubble_sort(records, num, false);
        quick_sort(records, 0, num - 1, false);
        end = clock();

        printf("After Sorts:\n");
        printKeys(records, num);
        printf("Sort time used: %4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    }

    return 0;
}