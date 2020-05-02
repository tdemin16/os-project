#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;
    int i; //Variabile usata per ciclare gli argomenti (argv[i])
    int value_return = 0; //Valore di ritorno
    int count = 0; //numero di file univoci da analizzare
    node filePath = NULL; //list of path's strings
    char *tmp;
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    node msg; //list used to pass path's to child

    char flag = FALSE; // se flag = true, l'argomento successivo è il numero o di n o di m
    char setn = FALSE; // se setn = true, n è stato cambiato
    char setm = FALSE; // se setn = true, m è stato cambiato
    FILE *fp;
    char riga[1035];

    //Variables for IPC
    int fd_1[2]; //Pipe
    int fd_2[2];
    pid_t f; //fork return value
    char args[8][7];

    if(argc < 1) { //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_A();
    }
    else {
        for(i = 1; i < argc && value_return == 0; i++) {
            if(!strcmp(argv[i], "-setn")) {//Check if argument is equal to -setn
                n = atoi(argv[i + 1]);
                if(n == 0) value_return = err_args_A();
                flag = TRUE;
                i++;
            }
            else if(!strcmp(argv[i], "-setm")) { //Check if argument is equal to -setm
                m = atoi(argv[i+1]);
                if(m == 0) value_return = err_args_A();
                flag = TRUE;
                i++;
            }else if(strncmp(argv[i], "-", 1) == 0){ //Se inizia per - ma non è setn/setm non è un input valido
                value_return = err_args_A();
            }

            if (flag == FALSE){ //Vuol dire che argv[i] e' un file o una cartella
                char command[strlen("find ") + strlen(argv[i]) + strlen(" -type f -follow -print" + 1)];
                strcpy(command, "find ");
                strcat(command, argv[i]);
                strcat(command, " -type f -follow -print");
                fp = popen(command, "r"); //avvia il comando e in fp prende l'output
                if (fp == NULL) 
                {
                    value_return = err_args_A();
                } else { //Il comando va a buon fine
                    while (fgets(riga, sizeof(riga), fp) != NULL) //Legge riga per riga e aggiunge alla lista
                {
                    char resolved_path[PATH_MAX];
                    realpath(riga, resolved_path);  //risalgo al percorso assoluto
                    resolved_path[strlen(resolved_path)-1] = 0; //tolgo l'ultimo carattere che manderebbe a capo                     
                    
                    
                    if (!(is_present(resolved_path, filePath))){
                        filePath = insert_first(resolved_path,filePath);
                        printf("[+] %s\n",resolved_path);
                        count++;
                    } else {
                        printf("[/] %s\n",resolved_path);
                    }
                    

                }
                pclose(fp);
                }
            } else {
                flag = FALSE;
            }


        }
        if(count == 0 && value_return == 0) value_return = err_args_A(); //counter is higher than zero, if not gives an error (value_return used to avoid double messages)
    }
    
    printf("\nNumero file: %d,n=%d m=%d\n",count,n,m);
    
 
/*
    //IPC
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_1) == -1) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di errore setta il valore di ritorno a ERR_PIPE
        }
    }
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_2) == -1) { //Controllo se nella creazione della pipe ci sono errori
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
                if (write(fd_1[WRITE], msg->path, sizeof(msg->path)) == -1) {
                    value_return = err_write();
                    //Capire cosa fare (killare tutto?)
                }
                msg = msg->next;
            }
            close(fd_1[WRITE]);
        }
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE
            printf("START son: %d\n", getpid());
            strcpy(args[0], "./C");
            strcpy(args[1], "-nfiles");
            sprintf(args[2], "%d");
            dup2(STDOUT_FILENO, fd_2[WRITE]); //close STDOUT_FILENO and open fd[WRITE]
            dup2(STDIN_FILENO, fd_1[READ]);
            close(fd_2[WRITE]);
            close(fd_1[READ]);
            execvp(args[0], args);
        }
    } 
    */
    return value_return;
}