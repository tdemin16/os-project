#include <stdio.h>
#include <stdlib.h>
//assert.h, ctype.h, errno.h, fcntl.h, float.h, limits.h, math.h, pthread.h, signal.h, stddef.h, string.h, sys/ipc.h, sys/msg.h, sys/shm.h, sys/stat.h, sys/types.h, sys/wait.h, time.h, unistd.h

#define DIM 1000

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int v[DIM];

void initialize_vector() //ha l'unico compito di settare tutte le ocorrenze a zero
{
    int i;
    for (i = 0; i < DIM; i++)
    {
        v[i] = 0;
    }
}

void print_vector() //U //stampa quante volte sono comparsi i caratteri CHE SONO PRESENTI
{
    int i;
    for (i = 0; i < DIM; i++)
    {
        if (v[i] != 0)
        {
            printf("\n%c è comparso %d volte", (i + 32), v[i]);
        }
    }
}

void set_add(char c) //Aggiunge al vettore definito globalmente la frequenza del char
{
    int val_ascii; 
    val_ascii = ((int)c) - 32; //prendo il valore ASCII dal carattere c e lo diminuisco di 32 (vedere intestazione per sapere perché)
    //printf("%d\n", val_ascii); //U
    v[val_ascii]++;
}

void get_subset(FILE *fp, int b, int e) //function that gets the chars from the
{
    char c;
    fseek(fp,b,SEEK_SET); //setto la posizione iniziale del cursore
    //fseek(fp,e,SEEK_END); //setto la posizione finale del cursore
    //while (!feof(fp)) //cycle
    while(e-b != 0) //P.S.: il primo carattere non è compreso, l'ultimo si
    {
        if (feof(fp))
        {
            printf("[!] Errore, sei andato oltre la fine del file (forse il punto di end è troppo alto?)");
            break;
        }
        else
        {
            fscanf(fp, "%1[^\n]%*[\n]", &c); //gets char
            printf("%c", c); //U
            set_add(c); //aggiunge al vettore delle frequenze il carattere c
            fflush(stdout);
        }
        b++;
    }
}

int main(int argc, char *argv[])
{
    initialize_vector();
    char *DIR = argv[1];       // setup the directory
    int begin = atoi(argv[2]); //setup start of the process in the file
    int end = atoi(argv[3]);   //setup end of the process in the file

    FILE *fp = fopen(DIR, "r"); //open in read mode the file in the directory
    if (fp == NULL)
    {
        printf("[!] Errore nell'apertura del file'");
        //exit(1); //serve(?)
    }

    get_subset(fp, begin, end); //getting all the chars

    print_vector();

    fclose(fp);
    printf("\n");
    return 1;
}


