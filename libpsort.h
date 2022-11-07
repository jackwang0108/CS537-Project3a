#ifndef _PSORT_H
#define _PSORT_H

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

// #define DEBUG

#define MAX_CHAR 50
#define SORT_SUCCESS 0
#define SORT_FAILURE 1
#define BYTE_PER_RECORD 100
#define SORT_THREAD_NUM 8
#define MAX_SORTED_JOBS 6

typedef struct _config{
    int sort_thread_num;
    int merge_thread_num;
    int record_num;
    int sorted_job_num;
    int record_per_thread;
} config;
extern config run_config;
void init_config();

#ifdef DEBUG
#define psort_error(s) _psort_error(s, __LINE__)
#else
#define psort_error(s) _psort_error("An error has occurred", __LINE__)
#endif
#define min(a, b) (a < b ? a : b)
#define delim printf("--------------------------------------\n");

/**
 * @brief 1个 byte 就是8位 bit，不考虑符号，所以 byte 就是 unsigned char
 * @brief 字节流 就是 byte 数组，所以 byteStream 就是 byte*
 * @brief 一个 记录 就是100个字节，所以 record 就是 byteStream*
 */
typedef unsigned char byte;
typedef byte *byteStream;
typedef byteStream *record_t;
// basic utils
void _psort_error(char *str, int lineno);
bool _is_little_endian();
char *byte2char(const byteStream buffer, int len);
static inline int get_key(record_t record);
void printKeys(record_t records[], int num);
int read_records(const char *filename, byteStream *buffer, int seek, int num);
int parse_records(byteStream buffer, record_t *records[], int byte);


/**
 * @brief get_key 从给定的record中读取key（前四个字节拼接得到的signed int）
 *
 * @param record 需要读取key的record
 * @return int record的key
 *
 * @author Shihong Wang
 * @date 2022.10.30
 */
static inline int get_key(record_t record)
{
    if (NULL == record)
        psort_error("record == NULL, didn't initialize?");
    return *(int *)(*record);
}


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
// sort_job functions
sort_job *sort_job_init(int sort_func, int seek, int num, bool reverse, char *filename);
int sort_job_release(sort_job *job);
int write_records(const char *filename, sort_job *job);


/**
 * @brief sorted_jobs是多个producer和多个consumer共享的数据，需要有一个互斥锁保证单独访问
 *
 * @brief sorted_jobs_mutex是sorted_jobs的锁
 *
 * @brief 此外，因为sorted_job可能满，也可能空，所以需要一个conditional variable来在producer和consumer之间相互通知
 */
extern int num_fill, front, rear;
extern sort_job **sorted_jobs;
extern pthread_cond_t sorted_jobs_cond;
extern pthread_mutex_t sorted_jobs_mutex;


// multi-thread functions
static inline bool is_full();
static inline bool is_empty();
static inline bool get_num();
void do_fill(sort_job *sorted_job);
sort_job *do_get();
void *sort_worker(void *arg);
void *append_worker(void *arg);
void *merge_worker(void *arg);


// Inline methods are supposed to be implemented in the header file. The compiler needs to know the code to actually inline it.
static inline bool is_empty(){
    return front == rear;
}

static inline bool is_full(){
    return (rear + 1) % MAX_SORTED_JOBS == front;
}

static inline bool get_num(){
    return num_fill;
}


// sort functions
static inline void swap(record_t *records, int i, int j);
static inline int less_than(record_t left, record_t right);
int bubble_sort(record_t records[], int num, bool reverse);
int _partition(record_t records[], int low, int high, bool reverse);
int _quick_sort(record_t records[], int low, int high, bool reverse);
int quick_sort(record_t records[], int num, bool reverse);
int order_merge(record_t old_records[], record_t new_records[], int num, int low, int mid, int high, bool reverse);
int _merge_sort(record_t records[], int num, bool reverse, int seg_start);
int merge_sort(record_t records[], int num, bool reverse);

#define BUBBLE_SORT 0
#define QUICK_SORT 1
#define MERGE_SORT 2

extern char *func_name[];
extern int (*sort_func[])(record_t *, int, bool);

#endif