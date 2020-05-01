#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    int n = 3;
    int m = 4;
    int i;
    int value_return = 0; //Valore di ritorno
    int count = 0; //numero di file univoci da analizzare
    node filePath = NULL; //list of path's strings
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    node msg; //list used to pass path's to child
    char* args [] = {"./C", "", NULL};

    char flag = FALSE; // se flag = true, l'argomento successivo è il numero o di n o di m
    char setn = FALSE; // quando 
    char setm = FALSE;

    //Variables for IPC
    int fd[2]; //Pipe
    pid_t f; //fork return value

    if (argc > 1) //APERTURA TRAMITE MAIN (ARGOMENTI)
    {

        system("clear");
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
        value_return = err_args(); //in caso di errore setta il valore di ritorno a ERR_ARGS
    }

    if(value_return == 0 && count == 0) {
        value_return = err_file();
    }

    //IPC
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd) == -1) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di errore setta il valore di ritorno a ERR_PIPE
        }
    }

    if(value_return == 0) { //same as before
        printf("FORK\n");
        f = fork(); //Fork dei processi
        if(f == -1) { //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork(); //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    if(value_return == 0) { //same
        msg = filePath; //copia il riferimento alla lista cosi' da poterla scorrere senza perdere i riferimanti effettivi
        if(f > 0) { //PARENT SIDE
            printf("START parent: %d\n", getpid());
            while(msg != NULL && value_return == 0) { //cicla su tutti gli elementi della lista
                if (write(fd[WRITE], msg->path, sizeof(msg->path)) == -1) {
                    value_return = err_write();
                    //Capire cosa fare (killare tutto?)
                }
                msg = msg->next;
            }
            close(fd[WRITE]);
        }
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE
            printf("START son: %d\n", getpid());
            sprintf(args[1], "%d", count);
            dup2(STDOUT_FILENO, fd[WRITE]);
            close(fd[READ]);
            close(fd[WRITE]);
            //execvp(args[0], args);
        }
    }

    return value_return;
}