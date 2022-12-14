#include "libpsort.h"

config run_config;

void init_config(int byte){
    int merger_vs_sorter = 10;
    int record_per_sort_thread = 1000;
    int max_sort_thread = 30;

    // Constant
    run_config.record_per_thread = 1000;
    run_config.record_num = byte / BYTE_PER_RECORD;

    run_config.sort_thread_num = run_config.record_num / record_per_sort_thread;
    if (run_config.sort_thread_num == 0 || run_config.record_num % record_per_sort_thread != 0)
        run_config.sort_thread_num += 1;
    
    // sort_thread up bound
    if (run_config.sort_thread_num > max_sort_thread){
        run_config.sort_thread_num = max_sort_thread;
        run_config.record_per_thread = run_config.record_num / run_config.sort_thread_num;
    }
    // run_config.sort_thread_num += 1;
    
    run_config.merge_thread_num = run_config.sort_thread_num / merger_vs_sorter;
    if (run_config.merge_thread_num == 0 || run_config.sort_thread_num % merger_vs_sorter != 0)
        run_config.merge_thread_num += 1;
    
    run_config.sorted_job_num = run_config.sort_thread_num + run_config.merge_thread_num + 1;
}

char *func_name[] = {
    [BUBBLE_SORT] = "bubble_sort",
    [QUICK_SORT] = "quick_sort",
    [MERGE_SORT] = "merge_sort",
};

int (*sort_func[])(record_t *, int, bool) = {
    [BUBBLE_SORT] = bubble_sort,
    [QUICK_SORT] = quick_sort,
    [MERGE_SORT] = merge_sort,
};

/**
 * @brief _psort_error用于输出错误信息，若定义DEBUG宏则输出错误行号, 和posort_error宏函数搭配使用
 *
 * @author Shihong Wang
 * @date 2022.10.29
 */
void _psort_error(char *str, int lineno)
{
    char *str_c;
#ifdef DEBUG
    // int to alphabetic-number
    char line[20];
    sprintf(line, "line: %d, ", lineno);
    //  construct str
    str_c = (char *)malloc(sizeof(char) * (strlen(str) + strlen(line)));
    sprintf(str_c, "%s", line);
    sprintf(str_c + strlen(line), "%s", str);
    printf("%s\n", str_c);
    exit(EXIT_FAILURE);
#else
    fprintf(stderr, "An error has occurred\n");
    exit(0);
#endif
}


/**
 * @brief _is_little_endian用于测试系统是否是小端序
 *
 * @return true 系统是小端序
 * @return false 系统不是小端序
 *
 * @author: Shihong Wang
 * @date: 2022.10.30
 */
bool _is_little_endian()
{
    volatile uint32_t i=0x01234567;
    // return 0 for big endian, 1 for little endian.
    return (*((uint8_t*)(&i))) == 0x67;
}


/**
 * @brief byte2char接受字节流, 而后返回字节流的16进制表示, 效果类似于xxd
 *
 * @param buffer 字节流, 类型是 byteStream (unsigned char *)
 * @param len 字节流长度
 * @return char* 16进制字符串形式的字节流
 *
 * @author Shihong Wang
 * @date 2022.10.30
 *
 * @bug fixed, unsigned char问题
 */
char *byte2char(const byteStream_t buffer, int len)
{
    int pos = 0;
    bool little_endian = !_is_little_endian();
    char *str = (char *)malloc(sizeof(char) * len * 2);
    if (NULL == str)
        psort_error("malloc fail");
    for (int i = 0; i < len; i++)
    {
        if (little_endian)
        {
            sprintf(&str[pos++], "%x", (unsigned int)(buffer[i] >> 4));
            sprintf(&str[pos++], "%x", (unsigned int)(buffer[i] & 0x0f));
        }
        else
        {
            sprintf(&str[pos++], "%x", (unsigned int)buffer[i] & 0x0f);
            sprintf(&str[pos++], "%x", (unsigned int)buffer[i] >> 4);
        }
    }
    return str;
}


/**
 *
 * @param records record数组
 * @param num record数组中的record数量
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
void printKeys(record_t records[], int num)
{
    if (NULL == records)
        psort_error("record is NULL, did not malloc records? Or worker thread didn't run first?");
    for (int i = 0; i < num; i++)
        printf("%4d -> %11d\n", i, get_key(records[i]));
}


/**
 * @brief read_records_worker以以二进制方式读取filename中数据并写入到buffer中
 *
 * @param filename 二进制文件名
 * @param buffer &(char *), 指向char *的指针的地址，不用提前malloc分配地址
 * @param seek 跳过多少个record
 * @param num 读取的record数, -1表示从seek读取到结束, -2读取全部
 * @return int 读取的字节数
 *
 * @author Shihong Wang
 * @date 2022.10.29
 */
int read_records(const char *filename, byteStream_t *buffer, int seek, int num)
{
    FILE *bin_file = fopen(filename, "rb");
    if (NULL == bin_file)
        psort_error("file open error");

    int byte = num * BYTE_PER_RECORD;
    if (num == -1)
    {
        fseek(bin_file, 0L, 2);
        byte = (int) ftell(bin_file) - (seek * BYTE_PER_RECORD);
    } else if (num == -2){
        fseek(bin_file, 0L, 2);
        byte = (int) ftell(bin_file);
    }
    if (byte <=0)
        psort_error("zero record");
    fseek(bin_file, (long)(seek * BYTE_PER_RECORD), 0);

    if (byte % 100 != 0)
        psort_error("record mismatch");

    *buffer = (byteStream_t)malloc(sizeof(char) * byte);
    if (NULL == *buffer)
        psort_error("malloc fail");
    if (fread(*buffer, 1, byte, bin_file) != byte)
        psort_error("record read fail");

    fclose(bin_file);
    return byte;
}


/**
 * @brief 将job中的record写入到filename中
 * 
 * @param filename 要写入的文件的文件名
 * @param job 要写入的job的名称
 * @return int 
 */
int write_records(const char* filename, sort_job_t*job){
    FILE *bin_file = fopen(filename, "wb");
    if (bin_file == NULL)
        psort_error("file open error");
    
    int writte_byte = 0;
    for (int i = 0; i < job->num; i++){
        if (fwrite(*(job->records[i]), BYTE_PER_RECORD, 1, bin_file) != 1)
            psort_error("record write error");
        writte_byte += BYTE_PER_RECORD;
    }
    return writte_byte;
}


/**
 * @brief parse_records接受二进制文件buffer，将解析后的record存入records数组中
 *
 * @param buffer 二进制文件内存地址
 * @param records record数组的地址
 * @param byte 二进制文件的字节数
 * @return int 解析得到的record数
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
int parse_records(byteStream_t buffer, record_t *records[], int byte)
{
    int num = byte / 100;

    *records = (record_t *)malloc(sizeof(record_t) * byte);
    if (NULL == *records)
        psort_error("malloc fail");

    for (int i = 0; i < num; i++)
    {
        (*records)[i] = (byteStream_t *)malloc(sizeof(byteStream_t));
        *(*records)[i] = (buffer + 100 * i);
    }
    return num;
}


/**
 * @brief less_than比较两个record的key，若左边小于右边，返回1，相等返回0，大于返回-1
 *
 * @param left 左侧的Record
 * @param right 右侧的Record
 * @return int 比较的结果
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
static inline int less_than(record_t left, record_t right)
{
    int left_key = get_key(left);
    int right_key = get_key(right);
    if (left_key < right_key)
        return 1;
    else if (left_key == right_key)
        return 0;
    else
        return -1;
}


/**
 * @brief swap用于交换record数组中指令的两个元素
 *
 * @param records record数组
 * @param i 第一个元素的index
 * @param j 第二个元素的index
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
static inline void swap(record_t *records, int i, int j)
{
    record_t temp = records[i];
    records[i] = records[j];
    records[j] = temp;
}


/**
 * @brief 冒泡排序，大保底
 *
 * @param records record数组
 * @param num record的数量
 * @param reverse 若为true，则按照降序排列，否则按照升序排列
 * @return int 状态码
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
int bubble_sort(record_t records[], int num, bool reverse)
{
    int sign = reverse == true ? 1 : -1;
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num - 1 - i; j++)
        {
            if (less_than(records[j], records[j + 1]) * sign >= 0)
                swap(records, j, j + 1);
        }
    }
    return SORT_SUCCESS;
}


/**
 * @brief 快速排序的partition函数
 *
 * @param records record数组
 * @param low 左侧index
 * @param high 右侧index
 * @param reverse 若为true，则按照降序排列，否则按照生序排列
 * @return int 新的pivot的index
 *
 * @author Shihong Wang
 * @date 2022.10.31
 */
int _partition(record_t records[], int low, int high, bool reverse)
{
    int pivot = low;
    record_t pivot_value = records[pivot];
    int sign = reverse == true ? -1 : 1;
    while (low < high)
    {
        while (low < high && less_than(pivot_value, records[high]) * sign >= 0)
            high--;
        records[low] = records[high];
        while (low < high && less_than(records[low], pivot_value) * sign >= 0)
            low++;
        records[high] = records[low];
    }
    records[low] = pivot_value;
    return low;
}


/**
 * @brief 快速排序母函数
 *
 * @param records record数组
 * @param low 左侧index
 * @param high 右侧index
 * @param reverse 若为true则降序排列，否则升序排列
 * @return int 程序状态码
 *
 * @author Shihong Wang
 * @date 2022.10.31
 */
int _quick_sort(record_t records[], int low, int high, bool reverse)
{
    if (low < high)
    {
        int pivot = _partition(records, low, high, reverse);
        _quick_sort(records, low, pivot - 1, reverse);
        _quick_sort(records, pivot + 1, high, reverse);
    }
    return SORT_SUCCESS;
}


/**
 * @brief 快速排序 wrapper，为了benchmark
 *
 * @param records record数组
 * @param num record的数量
 * @param reverse 若为true，则按照降序排列，否则按照升序排列
 * @return int 状态码
 *
 * @author Shihong Wang
 * @date 2022.10.31
 */
// Attention: 目前quick_sort运行明显慢于merge_sort，怀疑是递归版本的速度太慢了，后续改成循环版本
int quick_sort(record_t records[], int num, bool reverse)
{
    return _quick_sort(records, 0, num - 1, reverse);
}


/**
 * @brief order_merge 给定未排序的原数组和空的新数组，而后将low ~ mid 和 mid ~ high中的值按顺序排列到新数组中
 *
 * @param old_records 原数组
 * @param new_records 新数组
 * @param num 原数组中record数
 * @param low low的值
 * @param mid mid的值
 * @param high high的值
 * @param reverse 若为true，则为降序排列，否则为升序排列
 * @return int 状态码
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
int order_merge(record_t old_records[], record_t new_records[], int num, int low, int mid, int high, bool reverse)
{
    int k = low;
    int start1 = low, end1 = mid;
    int start2 = mid, end2 = high;
    while (start1 < end1 && start2 < end2)
    {
        if (!reverse)
            new_records[k++] = less_than(old_records[start1], old_records[start2]) >= 0 ? old_records[start1++] : old_records[start2++];
        else
            new_records[k++] = less_than(old_records[start1], old_records[start2]) >= 0 ? old_records[start2++] : old_records[start1++];
    }
    while (start1 < end1)
        new_records[k++] = old_records[start1++];
    while (start2 < end2)
        new_records[k++] = old_records[start2++];

    return 0;
}


/**
 * @brief 归并排序母函数
 *
 * @param records record数组
 * @param num record的数量
 * @param reverse 若为true，则按照降序排列，否则按照升序排列
 * @param seg_start 归并排序开始时每组中的记录数
 * @return int 状态码
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
int _merge_sort(record_t records[], int num, bool reverse, int seg_start)
{
    int sign = reverse == true ? -1 : 1;
    record_t *old_records = records;
    record_t *new_records = (record_t *)malloc(sizeof(record_t) * num);
    int seg, start;
    for (seg = seg_start; seg < num; seg += seg)
    {
        for (start = 0; start < num; start += seg * 2)
        {
            int low = start, mid = min(start + seg, num), high = min(start + seg * 2, num);
            order_merge(old_records, new_records, num, low, mid, high, reverse);
        }

        // swap for next sort
        record_t *temp = new_records;
        new_records = old_records;
        old_records = temp;
    }

    if (old_records != records)
    {
        for (int i = 0; i < num; i++)
            new_records[i] = old_records[i];
        new_records = old_records;
    }
    free(new_records);

    return SORT_SUCCESS;
}


/**
 * @brief 归并排序 wrapper, 为了benchmark
 *
 * @param records
 * @param num
 * @param reverse
 * @return int
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
int merge_sort(record_t records[], int num, bool reverse)
{
    return _merge_sort(records, num, reverse, 1);
}


int num_fill = 0;
int front = 0;
int rear = 0;
sort_job_t **sorted_jobs;
pthread_cond_t sorted_jobs_cond;
pthread_mutex_t sorted_jobs_mutex;


/**
 * @brief sort_job结构体的初始化函数
 *
 * @param sort_func 使用的sort function类型
 * @param seek 跳过多少个record
 * @param num 读取的record数, -1 表示读取全部
 * @param reverse 是否降序排列，若为true则降序排序
 * @param filename 需要读取的二进制文件文件名
 * @return sort_job* 指向结构体的指针
 *
 * @author Shihong Wang
 * @date 2022.11.2
 */
sort_job_t *sort_job_init(int sort_func, int seek, int num, bool reverse, char *filename)
{
    sort_job_t *job = (sort_job_t *)malloc(sizeof(sort_job_t));
    job->sort_func = sort_func;
    job->seek = seek;
    job->num = num;
    job->reverse = reverse;
    job->buffer = NULL;
    job->records = NULL;
    if (NULL == filename)
    {
        job->filename = NULL;
    }
    else
    {
        job->filename = (char *)malloc(sizeof(char) * strlen(filename));
        strcpy(job->filename, filename);
    }
    return job;
}


/**
 * @brief sort_job的析构函数
 *
 * @param job 指向sort_job的指针
 * @return int
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
int sort_job_release(sort_job_t *job)
{
    free(job->filename);
    if (job->buffer != NULL)
        free(job->buffer);
    if (job->records != NULL)
        free(job->records);
    free(job);
    return 0;
}


/**
 * @brief do_fill用于把sorted之后的job放入sorted_jobs队列中
 *
 * @param sorted_job 指向sorted_job的指针
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
void do_fill(sort_job_t *sorted_job)
{
    sorted_jobs[rear] = sorted_job;
    rear = (rear + 1) % MAX_SORTED_JOBS;
    num_fill++;
}


/**
 * @brief do_get用于从sorted_jobs队列中取出sorted_job
 *
 * @return sort_job* 指向sorted_job的指针
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
sort_job_t *do_get()
{
    sort_job_t *temp = sorted_jobs[front];
    front = (front + 1) % MAX_SORTED_JOBS;
    num_fill--;
    return temp;
}


/**
 * @brief sort_worker是排序线程执行的函数，每个sort_worker都是一个producer(产品放在sorted_queue中)
 *
 * @param arg 指向sort_job*的指针，传入前和使用前都需要进行类型强制转换
 * @return void*
 *
 * @author Shihong Wang
 * @date 2022.11.3
 */
void *sort_worker(void *arg)
{
    sort_job_t *job = (sort_job_t *)arg;
    int byte = read_records(job->filename, &job->buffer, job->seek, job->num);
    job->num = byte / 100;

    if (parse_records(job->buffer, &job->records, byte) != job->num)
    {
        char str[MAX_CHAR];
        sprintf(str, "thd -> %ld: parse_records mismatch!", (long)pthread_self());
        psort_error(str);
    }

    int result = sort_func[job->sort_func](job->records, job->num, job->reverse);
    if (result != SORT_SUCCESS)
    {
        char str[MAX_CHAR];
        sprintf(str, "thd -> %ld: sort fail!", (long)pthread_self());
        psort_error(str);
    }
    job->done = true;

    // 下面因为要访问共享资源sorted_jobs，所以都是临界区
    pthread_mutex_lock(&sorted_jobs_mutex);
    // 等待consumer
    while (is_full())
        pthread_cond_wait(&sorted_jobs_cond, &sorted_jobs_mutex);
    // 排序完当前job后，把job放入sorted_jobs_queue中，等待merger_worker处理，所以sort_worker就是producer
    do_fill(job);
    // 唤醒consumer
    pthread_cond_signal(&sorted_jobs_cond);
    pthread_mutex_unlock(&sorted_jobs_mutex);

    return (void *)0;
}

/**
 * @brief append_worker是append线程执行的函数，每个append_worker都是producer
 * 
 * @param arg (sort_job*)指向sort_job的结构体
 * @return void* 程序状态码
 * 
 * @author Shihong Wang
 * @date 2022.11.3
 */
void *append_worker(void *arg){
    sort_job_t *job = (sort_job_t*) arg;
    // 下面因为要访问共享资源sorted_jobs，所以都是临界区
    pthread_mutex_lock(&sorted_jobs_mutex);
    // Attention: 如果sorted_jobs已满(其他sort_worker/merge_worker线程填充)，且只有一个consumer，则此时会有卡住，所以要保证sorted_job有足够的容量，即sorted_job < sort_thread + merge_thread
    // 等待consumer
    while (is_full())
        pthread_cond_wait(&sorted_jobs_cond, &sorted_jobs_mutex);
    // 排序完当前job后，把job放入sorted_jobs_queue中，等待merger_worker处理，所以sort_worker就是producer
    do_fill(job);
    // 唤醒consumer
    pthread_cond_signal(&sorted_jobs_cond);
    pthread_mutex_unlock(&sorted_jobs_mutex);

    return (void*)0;
}


/**
 * @brief merge_worker是merge线程执行的函数，每个merge_worker是消费者
 *
 * @param arg 指向int的指针，用于判断是否已经排序完毕，调用前和使用前必须要进行强制类型转换
 * @return void*
 *
 * @bug #1 多个sort_worker，一个merge_worker，下述情况会发生死锁:
 *          merge_worker正在merge，而sorted_jobs已经全部运行完，并且填充满sorted_jobs，
 *          则此时由于sorted_jobs已经被填充满，故此时merge_worker等待consumer消耗sorted_jobs
 *          由于只有一个merge_worker，故此时就会卡在这里
 *      该bug修复方式是满足：sort_worker_thd + merge_worker_thd < max_sorted_jobs
 * 
 * @bug #2 多个sort_worker，一个merge_worker，下述情况会发生死锁:
 *          merge_worker在取完了sorted_job后，就从消费者变成了生产者，若此时有别的sort_worker填满了sorted_jobs
 *          后仍有sort_worker尝试do_fill，就会造成只有生产者而没有消费者的情况，此时merge_worker和sort_worker都会因为cv卡住
 *      该bug的修复方式就是将merge_worker拆分为sort_worker，重新插入的工作交给append_worker处理（子线程），以保证merge_worker不会等待
 * 
 * @bug #3 多个sort_worker，多个merge_worker，会发生死锁:
 *          已经没有了sort_worker，而所有的merge_worker都只有一个job，则此时发生死锁
 *          此bug类似于哲学家进餐问题，可以使用一个服务生（管理员）来管理、启动merge_worker
 *      该bug的修复方法就是等待一段时间后放弃merge，将job放回队列中，同时必须要有一个不会timeout的merge_thread
 *
 * @author Shihong Wang
 * @date 2022.11.4
 */
void *merge_worker(void *arg)
{   
    // receive arg
    bool wait = *(bool *)arg;

    bool last_job = false;
    // Notes: 不知道为什么，先malloc在free就不会报错，真的不理解为什么，难道是Copy-on-Write机制？真搞不明白
    sort_job_t *job = (sort_job_t *)malloc(sizeof(sort_job_t) * 1);
    free(job);
    job = NULL;

    // merge main
    while (true && !last_job)
    {   
        // merge once
        last_job = false;
        bool giveup = false, merge = true;
        int fill_ptr = 0;
        sort_job_t *jobs[2] = {NULL, NULL};
        while (fill_ptr < 2 && !last_job && !giveup)
        {   
            int state;
            struct timeval now;
            struct timespec expire;
            gettimeofday(&now, NULL);
            // Attention: 等待一会，否则放弃
            expire.tv_sec = now.tv_sec;
            expire.tv_nsec = now.tv_usec + 500000000;

            // try to get a sorted job
            // 下面因为要访问共享资源sorted_jobs，所以都是临界区
            pthread_mutex_lock(&sorted_jobs_mutex);
            // wait for producer
            while (is_empty()){
                if (wait)
                    state = pthread_cond_wait(&sorted_jobs_cond, &sorted_jobs_mutex);
                else
                    state = pthread_cond_timedwait(&sorted_jobs_cond, &sorted_jobs_mutex, &expire);
                // timeout, no producer give up 
                if (state != 0){
                    giveup = true;
                    for (int i = 0; i < 2; i++){
                        if (jobs[i] != NULL){
                            do_fill(jobs[i]);
                            jobs[i] = NULL;
                        }
                    }
                    fill_ptr = 0;
                    break;
                }
            }
            // get one job
            if (state == 0){
                jobs[fill_ptr++] = do_get();
            }
            // 唤醒consumer
            pthread_cond_signal(&sorted_jobs_cond);
            pthread_mutex_unlock(&sorted_jobs_mutex);

            // check if last job
            for (int i = 0; i < 2; i++){
                if (jobs[i] != NULL && jobs[i]->num >= run_config.record_num){
                    job = jobs[i];
                    merge = false, last_job = true;;
                    break;
                }
            }
        }

        // end thread
        if (giveup)
            break;

        if (merge)
        {
            // 归并排序
            int sum = jobs[0]->num + jobs[1]->num;
            job = sort_job_init(MERGE_SORT, 0, sum, false, (char *)NULL);

            record_t *temp_records = (record_t *)malloc(sizeof(record_t) * job->num);
            job->records = (record_t *)malloc(sizeof(record_t) * job->num);
            for (int i = 0, ptr = 0; i < 2; i++)
                for (int j = 0; j < jobs[i]->num; j++)
                    temp_records[ptr++] = jobs[i]->records[j];
            if (order_merge(temp_records, job->records, job->num, 0, jobs[0]->num, job->num, jobs[0]->reverse))
                psort_error("merge error!");
            // release job if job is merged by merge_worker
            for (int i = 0; i < 2; i++)
                if (jobs[i]->filename == NULL)
                    sort_job_release(jobs[i]);
            free(temp_records);
        }

        // 将新的job插入到循环队列尾
        pthread_t append_thd;
        pthread_create(&append_thd, NULL, append_worker, (void *)job);
    }

    return (void *)0;
}