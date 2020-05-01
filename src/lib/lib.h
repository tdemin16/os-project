#ifndef __LIB_H__
#define __LIB_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FALSE 0

typedef struct List{
    
    char* path;
    List* next;

}List;

List* init_list(char* p) {
    List* tmp = (List*)malloc(sizeof(List));
    tmp->path = p;
    tmp->next = NULL;

    return tmp;
}

List* insert_first(char* p, List* l) {
    //if l is null, it will produce the same result as initList
    //so it doesn't require a test
    List* tmp;
    tmp->path = p;
    tmp->next = l;

    return tmp;
}

//Boolean result
int is_present(char* p, List* l) {
    int ret = FALSE;
    List* tmp = l;

    if(tmp == NULL) {
        printf("Lista vuota\n");
    }
    else {
        while(tmp != NULL && !ret) {
            if(!strcmp(p, tmp->path)) {
                ret = TRUE;
            }
            tmp = tmp->next;
        }        
    }

    return ret;
}

#endif