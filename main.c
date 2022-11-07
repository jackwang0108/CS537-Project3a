#include "libpsort.h"

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
#define PRINTKEY 0

int main(int argc, char *argv[])
{

    byteStream buffer;

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
    struct timezone tz;
    struct timeval start, end;
    gettimeofday(&start, &tz);

    int byte = read_records(argv[1], &buffer, 0, -1);
    // printf("Test file: %s\n", argv[1]);
    // printf("%s -> %d bytes\n", argv[1], byte);
    // delim;

    // init all
    init_config(byte);
    pthread_mutex_init(&sorted_jobs_mutex, NULL);
    pthread_cond_init(&sorted_jobs_cond, NULL);
    sorted_jobs = (sort_job**)malloc(sizeof(sort_job*) * (run_config.sorted_job_num));
    pthread_t *sort_thread_pool = (pthread_t *)malloc(sizeof(pthread_t) * run_config.sort_thread_num);

    // start sort threads
    int seek = 0;
    int record_left = run_config.record_num;
    sort_job **jobs = (sort_job **)malloc(sizeof(sort_job *) * run_config.sort_thread_num);
    for (int j = 0; j < run_config.sort_thread_num; j++)
    {
        int num = run_config.record_per_thread;
        if (record_left < run_config.record_per_thread)
            num = -1;
        jobs[j] = sort_job_init(MERGE_SORT, seek, num, false, argv[1]);
        pthread_create(&sort_thread_pool[j], NULL, sort_worker, (void *)jobs[j]);
        seek += num;
        record_left -= num;
    }

    // start merge threads
    bool wait_arg = true;
    bool timeout_arg = false;
    pthread_t *merge_thread_pool = (pthread_t *)malloc(sizeof(pthread_t) * run_config.merge_thread_num);
    for (int j = 0; j < run_config.merge_thread_num; j++)
        if (j == 0)
            pthread_create(&merge_thread_pool[j], NULL, merge_worker, (void *)&wait_arg);
        else
            pthread_create(&merge_thread_pool[j], NULL, merge_worker, (void *)&timeout_arg);


    for (int j = 0; j < run_config.sort_thread_num; j++)
        pthread_join(sort_thread_pool[j], NULL);
    for (int j = 0; j < run_config.merge_thread_num; j++)
        pthread_join(merge_thread_pool[j], NULL);


    // get done_job
    pthread_mutex_lock(&sorted_jobs_mutex);
    // 等待producer
    while (is_empty())
        pthread_cond_wait(&sorted_jobs_cond, &sorted_jobs_mutex);
    sort_job* done_job = do_get();
    // 唤醒consumer
    pthread_cond_signal(&sorted_jobs_cond);
    pthread_mutex_unlock(&sorted_jobs_mutex);

    if (write_records(argv[2], done_job) != byte)
        psort_error("read record and write record mismatch");

    for (int j = 0; j < run_config.sort_thread_num; j++)
    {
        if (PRINTKEY == 1)
        {
            printf("Main, Thd -> %d: %s\n", j, func_name[jobs[j]->sort_func]);
            printf("After Sorts:\n");
            printKeys(jobs[j]->records, jobs[j]->num);
            delim;
        }
    }
    if (PRINTKEY == 1){
        printf("Main, After Merge:\n");
        // 下面因为要访问共享资源sorted_jobs，所以都是临界区
        printKeys(done_job->records, done_job->num);
        delim;
    }

    gettimeofday(&end, &tz);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    if (usec < 0){
        sec -= 1;
        usec += 1000000;
    }
    float used = sec + usec / 1000000.0;
    // printf("Main, sort %d records, using %.6f seconds\n", run_config.record_num, used);
#endif

    return 0;
}