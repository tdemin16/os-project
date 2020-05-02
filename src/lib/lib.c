#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

node insert_first(char *p, node l) {
    //if l is null, it will produce the same result as initList
    //so it doesn't require a test
    node tmp = (node)malloc(sizeof(node));

    tmp->path = p;
    tmp->next = l;

    return tmp;
}

//Boolean result
char is_present(char *p, node l) {
    char ret = FALSE;
    node tmp = l;

    if (tmp != NULL)
    {
        while (tmp != NULL && !ret)
        {
            if (!strcmp(p, tmp->path))
            {
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


///src/Analyzer/Q.c
//Initialize frequence vector all to 0
void initialize_vector(int v[]) {
    int i;
    for (i = 0; i < DIM_V; i++)
    {
        v[i] = 0;
    }
}

//Increase frequence of the global vector in the position val_ascii
void set_add(int v[], char c) {
    int val_ascii;
    val_ascii = ((int)c) - 32; //casting char to int and difference 32 (in order to save space on the vector)
    v[val_ascii]++;
}

//get the chars from the .txt files from the begin (b) to the end (e)
void get_subset(FILE *fp, int v[], int b, int e) {
    int i;
    char c;
    fseek(fp, b, SEEK_SET); //setting initial position of SEEK cursor
    for (i = b; i < e; i++) //P.S.: il primo carattere non è compreso, l'ultimo si
    {
        if (feof(fp))
        {
            err_end_file();
            break;
        }
        else
        {
            fscanf(fp, "%c", &c); //gets char
            printf("%c", c);      //display what you asked the process to analyze (uncomment to use)
            set_add(v, c);           //aggiunge al vettore delle frequenze il carattere c
        }
    }
}

//display how meny times chars are in the text (display only visited chars)
void print_vector(int v[])
{
    int i;
    for (i = 0; i < DIM_V; i++)
    {
        if (v[i] != 0)
        {
            printf("\n%c è comparso %d volte", (i + 32), v[i]);
        }
    }
}

///src/Analyzer/P.c
//return file length in terms of chars
int file_len(FILE* fp){
    int len = 0;
    char c;
    while(!feof(fp)){
        fscanf(fp,"%c",&c);
        len++;
        //printf("%d", len);
    }
    return len-1;
}

m_process* splitter(FILE *fp, int m)
{
    //This is for splitting the parts
    m_process* div = malloc(m);
    int file_length = file_len(fp);                        //get the length of the file in terms of chars
    int int_div = (file_length / m);                       //get the integer of the subdivision of the file
    double double_div = ((double)file_length / (double)m); //get the double of the subdivision of the file
    int rest = (double_div - int_div) * m;                 //get the number of chars that are out of consideration
    int i;
    for (i = 0; i < m; i++)
    {
        if (i == 0)
        {

            div[i].begin = 0;
        }
        else
        {
            div[i].begin = div[i - 1].end;
        }
        div[i].end = div[i].begin + int_div;
        //printf("\nRest: %d", rest);
        if (rest != 0)
        {
            div[i].end += 1;
            rest--;
        }
        div[i].part = i;
    }
    return div;
}

//Error handlers
int err_pipe() {
    printf("Errore nella creazione della pipe\n");
    return ERR_PIPE;
}

int err_args_A() {
    printf("\nErrore nella sintassi del comando.\nUsa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    return ERR_ARGS_A;
}

int err_args_C() {
    printf("\nErrore nella sintassi del comando.Usa:\n-nfiles <int> per indicare il numero di files (necessario)\n-setn <int> per settare n\n-setm <int> per settare m.\n\n");
    return ERR_ARGS_C;
}

int err_args_Q() {
    printf("[!] Errore nella sintassi del comando\nusa: ./Q inizio_analisi fine_analisi\n");
    return ERR_ARGS_Q;
}

int err_args_P() {
    printf("[!] Errore nella sintassi del comando\nusa: ./P m\n");
    return ERR_ARGS_Q;
}

int err_file() {
    printf("Errore, nessun file inserito\n");
    return ERR_FILE;
}
//Error if 3 arguments are inserted //Refers to Q.c
int err_fork() {
    printf("Errore, fork non riuscito\n");
    return ERR_FORK;
}

int err_write() {
    printf("Errore, write non riuscita\n");
    return ERR_WRITE;
}
//Error if fopen has failed //Refers to Q.c
int err_file_open(){
    printf("[!] Errore nell'apertura del file, analizzo fino alla fine del file\n");
    return ERR_FILE;
}

//Error if end point is over EOF //Refers to Q.c
int err_end_file(){
    printf("\n[!] Errore, sei andato oltre la fine del file\n");
    return ERR_FILE;
}