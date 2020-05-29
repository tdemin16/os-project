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
#include <time.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 95
#define DIM_RESP 2102
#define DIM_CMD 4096
#define DIM_PATH 4096

#define ERR_ARGS_A 1
#define ERR_PIPE 2
#define ERR_FILE 3
#define ERR_FORK 4
#define ERR_WRITE 5
#define ERR_FCNTL 6
#define ERR_EXEC 7
#define ERR_FIFO 8
#define ERR_CLOSE 9
#define ERR_KILL_PROC 10

#define REMOVED -2
#define INEXISTENCE -1
#define TO_BE_ANALYZED 0
#define ANALYZED 1
#define ANALYZED_INEXISTENCE 2

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

typedef struct process {
    int size;   //size of list process
    int *pid;   //vector containing all pids
    int count;  //counter to remember how many variables are inside the list
} process;

//Funzione di gestione della struttura processi e gestione chiusura
process *create_process(int);
void insertProcess(process *, pid_t);
void freeList(process *);
void initialize_processes(pid_t *, int);

//Funzioni di gestione della struttura PathList
array *createPathList(int);
void reallocPathList(array *, int);
char insertPathList(array *, char *, int);
int insertAndSumPathList(array *, char *, int);
char removeFromPathList(array *, char *c);
void freePathList(array *);
void resetPathList(array *);
int compare_mtime(array *, int, char *);
void cleanRemoved(array *);
char sameId(char *, char *);

//Parser e funzioni ausiliarie
int parser2(int, char **, array *, int *, int *, int *, int *, int *);
int parser_CheckArguments(int, char **, int *, int *);
int parser_LenghtCommand(char *);

//Funzioni IPC
void close_pipes(int *, int);
int unlock_pipes(int *, int);
char forkC(int *, int *, int *, int *);
char forkP(int *, int *, int *, int *);
char execC(int *, int *, int *, int *, int *, int *);
char execP(int *, int *, int *, int *, int *, int *);
int createPipe(int *, int);

//Funzioni Set on Fly e chiusura
void setOnFly(int, int, int *);
void setmOnFly(int, int *);
void mParseOnFly(char *, int *);
void parseOnFly(char *, int *, int *);
int parseSetOnFly(char *, int *, int *);
void nClearAndClose(int *, int);
void mSendOnFly(int *, int, int);
void closeAll(int *);
void nCleanSon(int *, int);

//Funzioni di elaborazione stringhe e vettori
int parse_string(char *, int *v);
void initialize_vector(long *);
void set_add(long *, char);
void get_subset(int *, long *, int, int);
void get_frequencies(int *, long *, int, int);
int file_len(FILE *);
void arrayToCsv(long *, char *);
int countDigit(int);
int lenghtCsv(int *);
char sumCsv(char *, char *);
void createCsv(long *, char *, char *);
char addCsvToArray(char *, long *);

//Funzioni di controllo percorsi
char fileExist(char *);

//Funzioni di controllo e parsing comandi
char checkArg(char *, int *);

//Funzioni di output
void printStat(char *);
void analyzeCluster(char *, char *);
void printCluster(char *);
void printInfoCluster();
void printHelp();
void printInfo();



//Errori
int err_file_open();
int err_pipe();
int err_args_A();
int err_overflow();
int err_input_A(char *);
int err_file();
int err_fork();
int err_write();
int err_fcntl();
int err_exec(int);
int err_fifo();
int err_close();
int err_kill_process_R();
int err_kill_process_A();
int err_kill_process_C();
int err_kill_process_P();
int err_kill_process_Q();

#endif