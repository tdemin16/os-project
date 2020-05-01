#include<stdio.h>
#include<stdlib.h>
#include "lib.h"

node init_list(char* p) {
    node tmp = (node)malloc(sizeof(List));
    tmp->path = p;
    tmp->next = NULL;

    return tmp;
}

node insert_first(char* p, node l) {
    //if l is null, it will produce the same result as initList
    //so it doesn't require a test
    node tmp;
    tmp->path = p;
    tmp->next = l;

    return tmp;
}

//Boolean result
char is_present(char* p, node l) {
    int ret = FALSE;
    node tmp = l;

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