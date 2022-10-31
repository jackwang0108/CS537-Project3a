#ifndef _PSORT_H
#define _PSORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG

#define MAX_FILENAME 256
#define RECORD_SIZE 100

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

/**
 * @brief read_record以以二进制方式读取filename中数据并写入到buffer中
 *
 * @param filename 二进制文件名
 * @param buffer &(char *), 指向char *的指针的地址，不用提前malloc分配地址
 * @return int 读取的字节数
 *
 * @author Shihong Wang
 * @date 2022.10.29
 */
int read_records(char *filename, byteStream *buffer)
{
    FILE *bin_file = fopen(filename, "rb");
    if (NULL == bin_file)
        psort_error("file open error");

    int byte = 0;
    fseek(bin_file, 0L, 2);
    byte = ftell(bin_file);
    fseek(bin_file, 0L, 0);

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
 * @param len 二进制文件的字节数
 * @return int 解析得到的record数
 * 
 * @author Shihong Wang
 * @date 2022.10.30
 */
int parse_records(byteStream buffer, record_t *records[], int len){
    int num = len / 100;

    *records = (record_t*) malloc(sizeof(record_t) * len);
    if (NULL == *records)
        psort_error("malloc fail");

    for (int i = 0; i < num; i++){
        (*records)[i] = (byteStream *) malloc(sizeof(byteStream));
        *(*records)[i] = (buffer + 100 * i);
    }
    return num;
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
    unsigned int temp = 0;
    for (int i = 0; i < 4; i++)
        temp = temp << 8 | ((int) (*record)[i]);
    return (int) temp;
}

/**
 * @brief compare比较两个record的key，若左边大于右边，返回1，相等返回0，小于返回-1
 * 
 * @param left 左侧的Record
 * @param right 右侧的Record
 * @return int 比较的结果
 * 
 * @author Shihong Wang
 * @date 2022.10.30
 */
int compare(record_t left, record_t right){
    int left_key = get_key(left);
    int right_key = get_key(right);
    if (left_key < right_key)
        return -1;
    else if (left_key == right_key)
        return 0;
    else
        return 1;
}

#endif