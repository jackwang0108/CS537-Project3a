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
        thread_pool = (pthread_t *) malloc(sizeof(pthread_t) * sort_thd_num);

        // TODO: quick sort还是有bug
        sort_job* job = sort_job_init(BUBBLE_SORT, 0, -1, false, argv[i]);
        // sort_job* job = sort_job_init(QUICK_SORT, 0, -1, false, argv[i]);

        printf("Main: %s\n", func_name[job->sort_func]);
        printf("Main, filename: %s\n", job->filename);
        start = clock();
        pthread_create(&thread_pool[0], NULL, sort_worker, (void *) job);
        pthread_join(thread_pool[0], NULL);
        end = clock();
        if (PRINTKEY == 1){
                printf("After Sorts:\n");
                printKeys(job->records, job->num);
        }
        printf("Sort time used: %4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
        delim;
#endif

    }

    return 0;
}