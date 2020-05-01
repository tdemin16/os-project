#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

node init_list(char *p)
{
    node tmp = (node)malloc(sizeof(List));
    tmp->path = p;
    tmp->next = NULL;

    return tmp;
}

node insert_first(char *p, node l)
{
    //if l is null, it will produce the same result as initList
    //so it doesn't require a test
    node tmp = (node)malloc(sizeof(node));
    tmp->path = p;
    tmp->next = l;

    return tmp;
}

//Boolean result
char is_present(char *p, node l)
{
    char ret = FALSE;
    node tmp = l;

    if (tmp == NULL)
    {
        printf("Lista vuota\n");
    }
    else
    {
        while (tmp != NULL && !ret)
        {
            if (!strcmp(p, tmp->path))
            {
                printf("##### %s - %s", p, tmp->path);
                ret = TRUE;
            }
            tmp = tmp->next;
        }
    }

    return ret;
}

//These are for /src/Analyzer/Q.c
//Initialize frequence vector all to 0
void initialize_vector()
{
    int i;
    for (i = 0; i < DIM_V; i++)
    {
        v[i] = 0;
    }
}
//Increase frequence of the global vector in the position val_ascii
void set_add(char c)
{
    int val_ascii;
    val_ascii = ((int)c) - 32; //casting char to int and difference 32 (in order to save space on the vector)
    v[val_ascii]++;
}
//get the chars from the .txt files from the begin (b) to the end (e)
void get_subset(FILE *fp, int b, int e) 
{
    int i;
    char c;
    fseek(fp, b, SEEK_SET); //setting initial position of SEEK cursor
    for (i = b; i < e; i++) //P.S.: il primo carattere non è compreso, l'ultimo si
    {
        if (feof(fp))
        {
            printf("[!] Errore, sei andato oltre la fine del file (forse il punto di end è troppo alto?)");
            break;
        }
        else
        {
            fscanf(fp, "%c", &c); //gets char
            printf("%c", c);      //display what you asked the process to analyze (uncomment to use)
            set_add(c);           //aggiunge al vettore delle frequenze il carattere c
        }
    }
}
//display how meny times chars are in the text (display only visited chars)
void print_vector()
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