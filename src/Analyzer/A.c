#include "../lib/lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[])
{
    int n = 3;
    int m = 4;

    char flag = FALSE;
    int i;
    if (argc > 1) //APERTURA TRAMITE MAIN (ARGOMENTI)
    {

        system("clear");
        //SimpleSet filePath; //creo e inizializzo set
        //set_init(&filePath);
        for (i = 1; i < argc; i++) //ciclo che controlla ogni argomento
        {
            if (strcmp("-setn", argv[i]) == 0)
            {                          //Controllo se si vuole cambiare n
                printf("\nset rilevato\n");
                n = atoi(argv[i + 1]); //presuppongo che l'argomento dopo sia un numero, bisogna aggiungere un controllo
                flag = TRUE; //il prossimo argomento è il valore di n o m, non va analizzato
                i++;
            }
            else
            {
                if (strcmp("-setm", argv[i]) == 0)
                {                          //Controllo se si vuole cambiare m
                    m = atoi(argv[i + 1]); //presuppongo che l'argomento dopo sia un numero, bisogna aggiungere un controllo
                    flag = TRUE; //il prossimo argomento è il valore di n o m, non va analizzato
                    i++;
                }
            }

            if (flag == FALSE)  //se è falso vuol dire che il prossimo argomento è un file o una cartella
            {
                printf("\n ANALIZZO: %s\n", argv[i]);
                char *analyze = argv[i]; //Costruzione comando
                int dim = strlen("find ") + strlen(analyze) + strlen(" -type f -follow -print" + 1);
                char command[dim];
                strcpy(command, "find ");
                strcat(command, analyze);
                strcat(command, " -type f -follow -print");

                FILE *fp;
                char riga[1035];

                fp = popen(command, "r"); //avvia il comando e in fp prende l'output
                if (fp == NULL)
                {
                    printf("Failed to run command\n");
                    exit(1);
                }
                int count = 0;
                while (fgets(riga, sizeof(riga), fp) != NULL) //Legge riga per riga e aggiunge al set
                {
                    char resolved_path[PATH_MAX];
                    realpath(riga, resolved_path);
                    count = count + 1;
                    printf("%d - %s", count, resolved_path);
                    //set_add(&filePath, resolved_path);
                }
                pclose(fp);
            }
            flag = FALSE;
        }
        //printf("\nFile da analizzare: %lu\nda dividere in n=%d e m=%d\n", set_length(&filePath),n,m);
        printf("\nn=%d e m=%d\n",n,m);
        //set_destroy(&filePath);
    }
    else //APERTURA MANUALE (SENZA ARGOMENTI)
    {
        printf("\nErrore nella sintassi del comando\nusa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    }

    return 0;
}