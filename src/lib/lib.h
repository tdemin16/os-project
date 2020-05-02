#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 100
#define ERR_ARGS_A 1
#define ERR_ARGS_C 2
#define ERR_PIPE 3
#define ERR_FILE 4
#define ERR_FORK 5
#define ERR_WRITE 6

struct List{
    char* path;
    struct List* next;
}List;

typedef struct List* node;

//List functions
node insert_first(char*, node);
char is_present(char*, node);
int count_list_elements(node);

///src/Analyzer/Q.c
void initialize_vector(int*);
void set_add(int*, char);
void get_subset(FILE*, int*, int, int);
void print_vector(int *);
int file_open_err();

///src/Analyzer/P.c
int file_len(FILE*);
int end_file_err();

//Error handlers
int err_pipe();
int err_args_A();
int err_args_C();
int err_file();
int err_fork();
int err_write();


#endif