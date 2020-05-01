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

    char flag = FALSE; // se flag = true, l'argomento successivo è il numero o di n o di m
    char setn = FALSE; // quando 
    char setm = FALSE;
    int i;
    if (argc > 1) //APERTURA TRAMITE MAIN (ARGOMENTI)
    {

        system("clear");
        node filePath = NULL;
        int count = 0;
        for (i = 1; i < argc; i++) //ciclo che controlla ogni argomento
        {
            if (!strcmp("-setn", argv[i]))
            { //Controllo se si vuole cambiare n
                n = atoi(argv[i + 1]); //presuppongo che l'argomento dopo sia un numero, bisogna aggiungere un controllo
                flag = TRUE;           //il prossimo argomento è il valore di n o m, non va analizzato
                i++;
            }
            else
            {
                if (!strcmp("-setm", argv[i]))
                {                          //Controllo se si vuole cambiare m
                    m = atoi(argv[i + 1]); //presuppongo che l'argomento dopo sia un numero, bisogna aggiungere un controllo
                    flag = TRUE;           //il prossimo argomento è il valore di n o m, non va analizzato
                    i++;
                }
            }

            if (flag == FALSE) //se è falso vuol dire che il prossimo argomento è un file o una cartella
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
                
                while (fgets(riga, sizeof(riga), fp) != NULL) //Legge riga per riga e aggiunge al set
                {
                    char resolved_path[PATH_MAX];
                    realpath(riga, resolved_path);
                    
                    
                    if (!(is_present(resolved_path, filePath))){
                        count = count + 1;
                        filePath = insert_first(resolved_path,filePath);
                        printf("\nNuovo inserimento\n");
                    } else {
                        printf("- E' gia' presente");
                    }
                    printf("%d - %s", count, resolved_path);
                }
                pclose(fp);
            }
            flag = FALSE;
        }
        //printf("\nFile da analizzare: %lu\nda dividere in n=%d e m=%d\n", set_length(&filePath),n,m);
        printf("\nn=%d e m=%d\n", n, m);
    }
    else //APERTURA MANUALE (SENZA ARGOMENTI)
    {
        printf("\nErrore nella sintassi del comando\nusa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    }

    return 0;
}