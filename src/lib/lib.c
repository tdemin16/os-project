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
    node tmp = (node)malloc(sizeof(node));
    tmp->path = p;
    tmp->next = l;

    return tmp;
}

//Boolean result
char is_present(char* p, node l) {
    char ret = FALSE;
    node tmp = l;

    if(tmp == NULL) {
        printf("Lista vuota\n");
    }
    else {
        while(tmp != NULL && !ret) {
            if(!strcmp(p, tmp->path)) {
                printf("##### %s - %s",p, tmp->path);
                ret = TRUE;
            }
            tmp = tmp->next;
        }        
    }

    return ret;
}

int count_list_elements(node l) {
    int val = 0;
    node tmp = l;

    while(tmp != NULL) {
        val++;
        tmp = tmp->next;
    }

    return val;
}

int err_pipe() {
    printf("Errore nella creazione della pipe\n");
    return ERR_PIPE;
}

int err_args() {
    printf("\nErrore nella sintassi del comando\nusa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    return ERR_ARGS;
}

int err_file() {
    printf("Errore, nessun file inserito\n");
    return ERR_FILE;
}

int err_fork() {
    printf("Errore, fork non riuscito\n");
    return ERR_FORK;
}

int err_write() {
    printf("Errore, write non riuscita\n");
    return ERR_WRITE;
}