#ifndef __LIB_H__
#define __LIB_H__

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 95
#define DIM_RESP 1051
#define DIM_CMD 4096
#define DIM_PATH 4112

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
#define ERR_FIFO 11
#define ERR_UNLINK 12
#define ERR_OPEN_PROC 13
#define ERR_SIGNAL 14
#define ERR_CLOSE 15
#define ERR_ARGS_R 16
#define ERR_ARGS_P 17
#define ERR_ENXIO 18
#define ERR_ARGS_M 19
#define ERR_KILL_PROC 20

#define REMOVED -2
#define INEXISTENCE -1
#define TO_BE_ANALIZED 0
#define ANALIZED 1
#define ANALIZED_INEXISTENCE 2

#define A 0
#define R 1

#define ADD 0
#define REMOVE 1

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

typedef struct {
    int size;  //size of array
    char **pathList;
    int *analyzed;
    int count;
    time_t *last_edit;
} array;

typedef struct m_process {
    int begin;
    int end;
    int part;
    //char *DIR; //Da usare per dare il file(???)
} m_process;

typedef struct process {
    int size;   //size of list process
    int *pid;   //vector containing all pids
    int count;  //counter to remember how many variables are inside the list
} process;

//functions for process to work
process *create_process(int);
void insertProcess(process *, pid_t);
void printList(process *);
void freeList(process *);

//Array struct functions -- sostituiscono lista (momentaneamente?)
array *createPathList(int);
void reallocPathList(array *, int);
char insertPathList(array *, char *, int);
int insertAndSumPathList(array *, char *, int);
char removeFromPathList(array *, char *c);
void printPathList(array *);
void freePathList(array *);
int dimPathList(array *);
void setAnalyzed(array *, int, int);
int getAnalyzed(array *, int);
void resetPathList(array *);
int compare_mtime(array *, int, char *);
void cleanRemoved(array *);

void close_pipes(int *, int);
int unlock_pipes(int *, int);

// /src/Analyzer/A.c
int parser(char, int, char **, array *, int *, int *, int *, int *);
int parser2(int, char **, array *, int *, int *, int *, int *);
int parser_CheckArguments(int, char **, int *, int *);
int parser_LenghtCommand(char *);
void parser_CreateCommand(char *);

void handle_sigint(int);
int parse_string(char *, int *v);
void add_process_to_v(pid_t, int *);
void initialize_processes(pid_t *, int);
char fileExist(char *);
void setOnFly(int, int, int *);
void setmOnFly(int, int *);
void closeAll(int *);

//parser A.c
char checkArg(char *, int *);

// /src/Analyzer/C.c
void parseOnFly(char *, int *, int *);
void mParseOnFly(char *, int *);
void nClearAndClose(int *, int);
void mSendOnFly(int *, int, int);
void forkC(int *, int *, int *, int *);
void forkP(int *, int *, int *, int *);
void execC(int *, int *, int *, int *, int *, int *);
void execP(int *, int *, int *, int *, int *, int *);
// /src/Analyzer/Q.c
void initialize_vector(int *);
void set_add(int *, char);
void get_subset(int *, int *, int, int);
void print_vector(int *);
void get_frequencies(int *, int *, int, int);
//int err_end_file(); used here
//int err_args_Q(); used here
//int err_file_open(); used here

// /src/Analyzer/P.c
int file_len(FILE *);
m_process *splitter(FILE *, int);
inline void swap(char *, char *);
char *reverse(char *, int, int);
char *itoa(int, char *, int);
void arrayToCsv(int *, char *);
char *integer_to_string(int);
int countDigit(int);
int lenghtCsv(int *);
char sumCsv(char *, char *);
void createCsv(int *, char *, char *);
char addCsvToArray(char *, int *);
char sameId(char *, char *);
//int err_args_P(); used here
//int err_file_open(); used here

// /src/R.c
void printStat(char *);
void analyzeCluster(char *, char *);
void printCluster(char *);
void printInfoCluster();
float roundValue(float, int);
void percAvanzamento(int, int);

//Error handlers
int err_file_open();
int err_pipe();
int err_end_file();
int err_args_A();
int err_overflow();
int err_input_A(char *);
int err_args_C();
int err_args_P();
int err_args_Q();
int err_file();
int err_fork();
int err_write();
int err_fcntl();
int err_exec(int);
int err_m_not_valid();
int err_part_not_valid();
int err_process_open(pid_t);
int err_fifo();
int err_unlink();
int err_signal();
int err_close();
int err_args_R();
int err_enxio();
int err_args_M();
int err_kill_process_R();
int err_kill_process_A();
int err_kill_process_C();
int err_kill_process_P();
int err_kill_process_Q();

#endif