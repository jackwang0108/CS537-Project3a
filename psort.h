#ifndef _PSORT_H
#define _PSORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG

#define MAX_FILENAME 256


/**
 * @brief psort_error用于输出错误信息，若定义DEBUG宏则输出错误行号
 * 
 * @author Shihong Wang
 * @date 2022.10.29
*/
void psort_error(char *str, int lineno){
    char *str_c;
    #ifdef DEBUG
        // int to alphabetic-number
        char line[20];
        sprintf(line, "line: %d, ", lineno);
        //  construct str
        str_c = (char *) malloc(sizeof(char) * (strlen(str) + strlen(line)));
        sprintf(str_c, "%s", line);
        sprintf(str_c + strlen(line), "%s", str);
    #else
        str_c = str;
    #endif

    printf("%s\n", str_c);
    exit(EXIT_FAILURE);
}

// TODO: 和XXD结果不同
char * byte2char(char *buffer, int len){
    char *str = (char *)malloc(sizeof(char) * len);
    for (int i = 0; i < len; i++){
        sprintf(&str[i], "%x", buffer[i]);
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
int read_records(char *filename, char** buffer){
    FILE *bin_file = fopen(filename, "rb");
    if (NULL == bin_file){
        psort_error("file open error", __LINE__);
        exit(EXIT_FAILURE);
    }

    int byte = 0;
    fseek(bin_file, 0L, 2);
    byte = ftell(bin_file);
    fseek(bin_file, 0L, 0);

    if (byte % 100 != 0){
        psort_error("record mismatch", __LINE__);
    }

    *buffer = (char *)malloc(sizeof(char) * byte);
    if (fread(*buffer, 1, byte, bin_file) != byte)
        psort_error("record read fail", __LINE__);

    return byte;
}


#endif