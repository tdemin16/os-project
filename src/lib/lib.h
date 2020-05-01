#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0

struct List{
    char* path;
    struct List* next;
}List;

typedef struct List* node;

node init_list(char* p);
node insert_first(char* p, node l);
char is_present(char* p, node l);

#endif