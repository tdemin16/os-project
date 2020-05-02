#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include <sys/types.h>
#include <unistd.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 100
#define ERR_ARGS 1
#define ERR_PIPE 2
#define ERR_FILE 3
#define ERR_FORK 4
#define ERR_WRITE 5

struct List{
    char* path;
    struct List* next;
}List;

typedef struct List* node;

//List functions
node init_list(char*);
node insert_first(char*, node);
char is_present(char*, node);
int count_list_elements(node);

///src/Analyzer/Q.c
void initialize_vector(int*);
void set_add(int*, char);
void get_subset(FILE*, int*, int, int);
void print_vector(int *);

///src/Analyzer/P.c
int file_len(FILE* fp);

//Error handlers
int err_pipe();
int err_args();
int err_file();
int err_fork();
int err_write();

#endif