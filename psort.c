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


// #define BENCHMARK
#define MAIN
#define PRINTKEY 1

int main(int argc, char* argv[]){

    byteStream buffer;
    for (int i = 1; i < argc; i++){
        clock_t start, end;

        int byte = read_records(argv[i], &buffer, 0, -1);
        printf("Test file: %s\n", argv[i]);
        printf("%s -> %d bytes\n", argv[i], byte);
        delim;

#ifdef BENCHMARK
        // Code for benchmark
        record_t *records;
        int num = parse_records(buffer, &records, byte);
        for (int j = 0; j < sizeof(sort_func) / sizeof(sort_func[0]); j++){
            printf("Benchmark: %s\n", func_name[j]);
            start = clock();
            sort_func[j](records, num, false);
            end = clock();
            if (PRINTKEY == 1){
                printf("After Sorts:\n");
                printKeys(records, num);
            }
            printf("Sort time used: %4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
            delim;
        }
#endif

#ifdef MAIN
        // Code for parallel sort
        int sort_thd_num = infer_thread_num();
        sort_job* jobs = (sort_job*) malloc(sizeof(jobs) * sort_thd_num);
        thread_pool = (pthread_t *) malloc(sizeof(pthread_t) * sort_thd_num);

        jobs[0].filename = (char *) malloc(sizeof(char) * strlen(argv[i]));
        strcpy(jobs[0].filename, argv[1]);
        jobs[0].seek = 0;
        jobs[0].num = -1;
        jobs[0].reverse = false;
        jobs[0].sort_func = 1;

        // BUG: Main中的filename和传入sort_worker的filename不一样，但是调试时间长了又没问题了(F5到断点等一会），推测是因为pthread调度的问题
        // TODO: 修上面这个BUG
        printf("Main: %s\n", func_name[jobs[0].sort_func]);
        printf("Main, filename: %s\n", jobs[0].filename);
        start = clock();
        pthread_create(&thread_pool[0], NULL, sort_worker, (void *) jobs);
        pthread_join(thread_pool[0], NULL);
        end = clock();
        if (PRINTKEY == 1){
                printf("After Sorts:\n");
                printKeys(jobs[0].records, jobs[0].num);
        }
        printf("Sort time used: %4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
        delim;
#endif

    }

    return 0;
}