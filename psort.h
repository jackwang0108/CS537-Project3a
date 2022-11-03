#ifndef _PSORT_H
#define _PSORT_H

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define DEBUG

#define MAX_THREAD 1
#define MAX_CHAR 50
#define BYTE_PER_RECORD 100
#define SORT_SUCCESS 0
#define SORT_FAILURE 1

#define delim printf("--------------------------------------\n");
#define psort_error(s) _psort_error(s, __LINE__)

typedef unsigned char byte;
typedef byte* byteStream;
typedef byteStream* record_t;

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
#else
    str_c = str;
#endif

    printf("%s\n", str_c);
    exit(EXIT_FAILURE);
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
    int i = 1;
    char *a = (char *)&i;
    if (*a == 1)
        return 1;
    return 0;
}

/**
 * @brief get_key 从给定的record中读取key（前四个字节拼接得到的signed int）
 * 
 * @param record 需要读取key的record
 * @return int record的key
 * 
 * @author Shihong Wang
 * @date 2022.10.30
 */
int get_key(record_t record){
    return *(int *)(*record);
    // 下面的计算方式有问题，忽略了大端序和小端序
    // unsigned int temp = 0;
    // for (int i = 0; i < 4; i++)
    //     temp = temp << 8 | ((int) (*record)[i]);
    // return (int) temp;
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
char* byte2char(const byteStream buffer, int len)
{
    int pos = 0;
    bool little_endian = _is_little_endian();
    char *str = (char *)malloc(sizeof(char) * len * 2);
    if (NULL == str)
        psort_error("malloc fail");
    for (int i = 0; i < len; i++)
    {
        if (little_endian){
            sprintf(&str[pos++], "%x", (unsigned int) (buffer[i] >> 4));
            sprintf(&str[pos++], "%x", (unsigned int) (buffer[i] & 0x0f));
        } else {
            sprintf(&str[pos++], "%x", (unsigned int) buffer[i] & 0x0f);
            sprintf(&str[pos++], "%x", (unsigned int) buffer[i] >> 4);
        }
    }
    return str;
}

void printKeys(record_t records[], int num){
    for (int i = 0; i < num; i++)
        printf("%4d -> %11d\n", i, get_key(records[i]));
}

/**
 * @brief read_records_worker以以二进制方式读取filename中数据并写入到buffer中
 *
 * @param filename 二进制文件名
 * @param buffer &(char *), 指向char *的指针的地址，不用提前malloc分配地址
 * @param seek 跳过多少个record
 * @param num 读取的record数, -1 表示读取全部
 * @return int 读取的字节数
 *
 * @author Shihong Wang
 * @date 2022.10.29
 */
int read_records(char *filename, byteStream *buffer, int seek, int num)
{
    FILE *bin_file = fopen(filename, "rb");
    if (NULL == bin_file)
        psort_error("file open error");

    int byte = num * BYTE_PER_RECORD;
    if (num < 0){
        fseek(bin_file, 0L, 2);
        byte = ftell(bin_file);
    }
    fseek(bin_file, (long) (seek * BYTE_PER_RECORD), 0);

    if (byte % 100 != 0)
        psort_error("record mismatch");

    *buffer = (byteStream)malloc(sizeof(char) * byte);
    if (NULL == *buffer)
        psort_error("malloc fail");
    if (fread(*buffer, 1, byte, bin_file) != byte)
        psort_error("record read fail");

    return byte;
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
int parse_records(byteStream buffer, record_t *records[], int byte){
    int num = byte / 100;

    *records = (record_t*) malloc(sizeof(record_t) * byte);
    if (NULL == *records)
        psort_error("malloc fail");

    for (int i = 0; i < num; i++){
        (*records)[i] = (byteStream *) malloc(sizeof(byteStream));
        *(*records)[i] = (buffer + 100 * i);
    }
    return num;
}


/**
 * @brief compare比较两个record的key，若左边小于右边，返回1，相等返回0，大于返回-1
 * 
 * @param left 左侧的Record
 * @param right 右侧的Record
 * @return int 比较的结果
 * 
 * @author Shihong Wang
 * @date 2022.10.30
 */
int less_than(record_t left, record_t right){
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
void swap(record_t *records, int i, int j){
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
int bubble_sort(record_t records[], int num, bool reverse){
    int sign = reverse == true ? 1 : -1;
    for (int i = 0; i < num; i++){
        for (int j = 0; j < num - 1 - i; j++){
            if (less_than(records[j], records[j+1]) * sign >= 0)
                swap(records, j, j+1);
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
int _partition(record_t records[], int low, int high, bool reverse){
    int pivot = low;
    record_t pivot_value = records[pivot];
    int sign = reverse == true ? -1 : 1;
    while (low < high)
    {
        while (low < high && less_than(records[pivot], records[high]) * sign > 0)
            high--;
        records[low] = records[high];
        while (low < high && less_than(records[low], records[pivot]) * sign >= 0)
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
int _quick_sort(record_t records[], int low, int high, bool reverse){
    if (low < high){
        int pivot = _partition(records, low, high, reverse);
        _quick_sort(records, low, pivot - 1, reverse);
        _quick_sort(records, pivot + 1, high, reverse);
    }
    return SORT_SUCCESS;
}


/**
 * @brief 快速排序 warpper，为了benchmark
 * 
 * @param records record数组
 * @param num record的数量
 * @param reverse 若为true，则按照降序排列，否则按照升序排列
 * @return int 状态码
 * 
 * @author Shihong Wang
 * @date 2022.10.31
 */
int quick_sort(record_t records[], int num, bool reverse){
    return _quick_sort(records, 0, num - 1, reverse);
}


#define BUBBLE_SORT 0
#define QUICK_SORT 1

char *func_name[] = {
    [BUBBLE_SORT] = "bubble_sort",
    [QUICK_SORT] = "quick_sort",
};

int (*sort_func[])(record_t *, int, bool) = {
    [BUBBLE_SORT] = bubble_sort,
    [QUICK_SORT] = quick_sort,
};


pthread_t *thread_pool;

typedef struct _sort_job
{
    // Notes: init 必须指定
    int sort_func;
    int seek;
    int num;
    bool reverse;
    char *filename;

    // Notes: done, records和buffer由worker填充
    bool done;
    record_t *records;
    byteStream buffer;
} sort_job;

/**
 * @brief infer_thread_num 用于推断需要多少个线程进行排序
 * 
 * @return int 需要的线程数量
 */
// TODO 写完这个函数
int infer_thread_num(){
    return MAX_THREAD;
}

/**
 * @brief sort_job结构体的初始化函数
 * 
 * @param sort_func 
 * @param seek 
 * @param num 
 * @param done 
 * @param reverse 
 * @param filename 
 * @return sort_job* 指向结构体的指针
 */
sort_job* sort_job_init(int sort_func, int seek, int num, bool reverse, char* filename){
    sort_job *job = (sort_job *) malloc(sizeof(sort_job));
    job->sort_func = sort_func;
    job->seek = seek;
    job->num = num;
    job->reverse = reverse;
    job->filename = (char *) malloc(sizeof(char) * strlen(filename));
    strcpy(job->filename, filename);
    return job;
}

/**
 * @brief sort_job的析构函数
 * 
 * @param job 指向sort_job的指针
 * @return int 
 */
int sort_job_release(sort_job* job){
    free(job->filename);
    free(job->buffer);
    free(job->records);
    free(job);
    return 0;
}

/**
 * @brief sort_word是排序线程执行的函数
 * 
 * @param job 
 * @param bin_file 
 * @return void* 
 */
void* sort_worker(void *arg){
    sort_job *job = (sort_job *) arg;
    printf("thd -> %ld: filename: %s\n", (long) pthread_self(), job->filename);
    int byte = read_records(job->filename, &job->buffer, job->seek, job->num);
    job->num = byte / 100;

    if(parse_records(job->buffer, &job->records, byte) != job->num){
        char str[MAX_CHAR];
        sprintf(str, "thd -> %ld: parse_records mismatch!", (long) pthread_self());
        psort_error(str);
    }

    int result = sort_func[job->sort_func](job->records, job->num, job->reverse);
    if (result != SORT_SUCCESS){
        char str[MAX_CHAR];
        sprintf(str, "thd -> %ld: sort fail!", (long) pthread_self());
        psort_error(str);
    }
    job->done = true;

    return (void*) 0;
}

#endif