#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<math.h>
#include<fcntl.h>


#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 95
#define ERR_ARGS_A 1
#define ERR_ARGS_C 2
#define ERR_ARGS_Q 3
#define ERR_PIPE 4
#define ERR_FILE 5
#define ERR_FORK 6
#define ERR_WRITE 7
#define ERR_FCNTL 8
#define ERR_EXEC 9
#define ERR_DATA 10

struct List{
    char* path;
    struct List* next;
}List;

typedef struct m_process{
    int begin;
    int end;
    int part;
    //char *DIR; //Da usare per dare il file(???)
}m_process;

typedef struct List* node;

//List functions
node insert_first(char*, node);
char is_present(char*, node);
int count_list_elements(node);

void close_pipes(int*, int);
int unlock_pipes(int*, int);

// /src/Analyzer/A.c
int parse_string(char*, int* v); 

// /src/Analyzer/Q.c
void initialize_vector(int*);
void set_add(int*, char);
void get_subset(FILE*, int*, int, int);
void print_vector(int*);
//int err_end_file(); used here
//int err_args_Q(); used here
//int err_file_open(); used here

// /src/Analyzer/P.c
int file_len(FILE*);
m_process* splitter(FILE*,int);
inline void swap(char*, char*);
char* reverse(char*, int, int);
char* itoa(int, char*, int);
//int err_args_P(); used here
//int err_file_open(); used here

// /src/R.c
void printStat(char *);
void printStat_Cluster(char *);


//Error handlers
int err_file_open();
int err_pipe();
int err_end_file();
int err_args_A();
int err_input_A(char*);
int err_args_C();
int err_args_P();
int err_args_Q();
int err_file();
int err_fork();
int err_write();
int err_fnctl();
int err_exec(int);
int err_m_not_valid();


#endif