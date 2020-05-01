#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0
#define DIM_V 100

int v[DIM_V]; //For Q.c

struct List{
    char* path;
    struct List* next;
}List;

typedef struct List* node;

node init_list(char* p);
node insert_first(char* p, node l);
char is_present(char* p, node l);

//These are for /src/Analyzer/Q.c
void initialize_vector();
void set_add(char c);
void get_subset(FILE *fp, int b, int e);
void print_vector();

#endif