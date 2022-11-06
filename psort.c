#include "psort.h"

// Notes:
//    目前的打算是分块读取文件，然后每个块内使用快排/Alpha排序……排序方法
//    然后每个块使用一个单独的线程来排序
//    每个块排序完毕之后再使用归并排序或者其他递归排序方法进行排序
//    sort_worker处理sort_job，而后生成merge_job，merge_worker进行归并排序，中间有一个merge_queue
//    标准的多consumer、producer模型

// Attention:
//    估计测试文件会很大，不然没有办法有效地测试性能
//    所以我严重怀疑等截止日期快到的时候实验机器会变得极其卡顿

// Warning:
//    1. 使用tmux和printKey在一起有BUG，输出不全而且可能会出错（record数量 > 1000时），初步猜测是tmux的字符缓冲区有问题，来不及刷新
// *  2. 关于pthread调度问题，pthread是在主线程运行一段时间之后才会运行子线程，所以如果子线程还没有给record分配内存，但是主线程又已经printKey，就会报错

// Notes:
//    测试排序函数注释掉MAIN宏，使用BENCHMARK宏, 测试Parallel Sort则注释掉BENCHMARK宏，使用MAIN宏

// TODO: 使用mmap快速读取文件

// #define BENCHMARK
#define MAIN
#define PRINTKEY 1

int main(int argc, char *argv[])
{

    byteStream buffer;
    for (int i = 1; i < argc; i++)
    {

#ifdef BENCHMARK
        // Code for benchmark
        clock_t start, end;

        int byte = read_records(argv[i], &buffer, 0, -1);
        printf("Test file: %s\n", argv[i]);
        printf("%s -> %d bytes\n", argv[i], byte);
        delim;
        record_t *records;
        int num = parse_records(buffer, &records, byte);
        for (int j = 0; j < sizeof(sort_func) / sizeof(sort_func[0]); j++)
        {
            printf("Benchmark: %s\n", func_name[j]);
            start = clock();
            sort_func[j](records, num, false);
            end = clock();
            if (PRINTKEY == 1)
            {
                printf("After Sorts:\n");
                printKeys(records, num);
            }
            printf("Sort time used: %4f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
            delim;
        }
#endif

#ifdef MAIN
        // Code for parallel sort
        int byte = read_records(argv[i], &buffer, 0, -1);
        printf("Test file: %s\n", argv[i]);
        printf("%s -> %d bytes\n", argv[i], byte);
        delim;

        int max_record = byte / BYTE_PER_RECORD;
        int sort_thd_num = infer_thread_num(byte);
        // int merge_thd_num = 3;
        int merge_thd_num = sort_thd_num / 4;
        sorted_jobs = (sort_job**)malloc(sizeof(sort_job*) * (sort_thd_num + merge_thd_num + 1));

        // init mutual exclusion (Mutex) and conditional variable (cv)
        pthread_mutex_init(&sorted_jobs_mutex, NULL);
        pthread_cond_init(&sorted_jobs_cond, NULL);
        pthread_t *sort_thread_pool = (pthread_t *)malloc(sizeof(pthread_t) * sort_thd_num);

        // start sort threads
        int seek = 0;
        int num = RECORD_PER_THREAD;
        sort_job **jobs = (sort_job **)malloc(sizeof(sort_job *) * sort_thd_num);
        for (int j = 0; j < sort_thd_num; j++)
        {
            jobs[j] = sort_job_init(MERGE_SORT, seek, num, false, argv[i]);
            pthread_create(&sort_thread_pool[j], NULL, sort_worker, (void *)jobs[j]);
            seek += num;
        }

        // start merge threads
        merge_arg wait_arg = {max_record, false};
        merge_arg timeout_arg = {max_record, true};
        pthread_t *merge_thread_pool = (pthread_t *)malloc(sizeof(pthread_t) * merge_thd_num);
        for (int j = 0; j < merge_thd_num; j++)
            if (j == 0)
                pthread_create(&merge_thread_pool[j], NULL, merge_worker, (void *)&wait_arg);
            else
                pthread_create(&merge_thread_pool[j], NULL, merge_worker, (void *)&timeout_arg);


        for (int j = 0; j < sort_thd_num; j++)
            pthread_join(sort_thread_pool[j], NULL);
        for (int j = 0; j < merge_thd_num; j++)
            pthread_join(merge_thread_pool[j], NULL);

        for (int j = 0; j < sort_thd_num; j++)
        {
            printf("Main, Thd -> %d: %s\n", j, func_name[jobs[j]->sort_func]);
            if (PRINTKEY == 1)
            {
                printf("After Sorts:\n");
                printKeys(jobs[j]->records, jobs[j]->num);
            }
            delim;
        }
        if (PRINTKEY == 1){
            printf("Main, After Merge:\n");
            // 下面因为要访问共享资源sorted_jobs，所以都是临界区
            pthread_mutex_lock(&sorted_jobs_mutex);
            // 等待producer
            while (is_empty())
                pthread_cond_wait(&sorted_jobs_cond, &sorted_jobs_mutex);
            sort_job* done_job = do_get();
            // 唤醒consumer
            pthread_cond_signal(&sorted_jobs_cond);
            pthread_mutex_unlock(&sorted_jobs_mutex);
            printKeys(done_job->records, done_job->num);
            delim;
        }


#endif
    }

    return 0;
}