#include "./lib/lib.h"

int check_command(char *str);  //controlla il comando un numero corrispondente: -1 comando non riconosciuto ecc

process *p;  //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)

void handle_sigint(int sig) {
    printf("\n[!] Ricevuta terminazione da M, inizio terminazione processo A ... \n");
    int i = p->count - 1;  //start from the end
    while (i != 0)         //while we haven't controlled every single process
    {
        if (p->pid[i] > 0)  //Processo padre
        {
            if (kill(p->pid[i], 9) == 0) {                                     //Tries to kill process with pid saved in pid[i]
                printf("\tProcesso %d terminato con successo!\n", p->pid[i]);  //if it success you terminated it correctly
            } else {
                printf("\t[!] Errore, non sono riuscito a chiudere il processo %d!", p->pid[i]);  //if it fail something is wrong
            }
        }
        i--;  //i-- otherwise it will go to infinity
    }
    freeList(p);  //free memory allocated for p
    printf("[!] ... Chiusura processi terminata\n");
    exit(-1);  //return exit with error -1
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);  //Handler for SIGINT (Ctrl-C)

    //------------------------------ DEFINIZIONE VARIABILI
    p = create_process(1);  //Allocate dynamically p with dimension 1
    int value_return = 0;
    pid_t f = getpid();
    char cmd[DIM_CMD];  //contiente il comando lanciato
    char ch = '0';      //per leggere un carattere per volta
    int end = FALSE;    //TRUE se lanciato il comando "close"
    int res_cmd;
    int _write = TRUE;
    int i;
    int id;

    //IPC Variables--------------------------------------------------
    int fd[4];
    char array_args[4];

    if (argc == 1) {
        value_return = err_args_M();
    }

    //IPC
    if (value_return == 0) {
        for (i = 0; i < 3; i += 2) {
            if (pipe(fd + i) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                value_return = err_pipe();  //In caso di errore setta il valore di ritorno
            }
        }
    }

    //Set Non-blocking pipes (Shouldn't block anyway, just to be sure)
    if (value_return == 0) {
        if (unlock_pipes(fd, 4) == -1) {
            value_return = err_fcntl();
        }
    }
    insertProcess(p, getpid());
    if (value_return == 0) {
        for (i = 0; i < 2 && f > 0 && value_return == 0; i++) {
            f = fork();
            if (f == 0)
                id = i;
            else if (f == -1)
                value_return = err_fork();
            else
                insertProcess(p, f);
        }
    }

    //--------------------------------------------------------------------------------
    if (value_return == 0) {
        if (f > 0) {                             //PARENT SIDE
            while (!end && value_return == 0) {  //--------- CICLO DI ATTESA COMANDI IN INPUT
                strcpy(cmd, "");                 //svuota la stringa per il prossimo comando
                //printf("> ");
                fflush(stdout);
                ch = '\0';
                while (ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
                    ch = getc(stdin);
                    if (ch != '\n') strcat(cmd, &ch);  //concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
                }

                res_cmd = check_command(cmd);
                if (res_cmd == -1) {
                    //richiama la funzione help() coi comandi
                    printf("Comando inserito non corretto\n");
                }
                if (res_cmd == 0) end = TRUE;  //con il comando close interrompe il ciclo
                if (res_cmd == 1) {
                    while (value_return == 0 && _write) {
                        if (write(fd[R * 2 + WRITE], cmd, DIM_CMD) == -1) {
                            if (errno != EAGAIN) {
                                value_return = err_write();
                            }
                        } else {
                            _write = FALSE;
                        }
                    }
                    _write = TRUE;
                }
                if (res_cmd == 2) {
                    while (value_return == 0 && _write) {
                        if (write(fd[A * 2 + WRITE], cmd, DIM_CMD) == -1) {
                            if (errno != EAGAIN) {
                                value_return = err_write();
                            }
                        } else {
                            _write = FALSE;
                        }
                    }
                    _write = TRUE;
                }
            }
            close(fd[R * 2 + WRITE]);
            close(fd[R * 2 + READ]);
            close(fd[A * 2 + WRITE]);
            close(fd[A * 2 + READ]);
            printf("Closing...\n");
        }
    }

    if (value_return == 0) {
        if (id == A && f == 0) {  //A SIDE

            //Change binary file
            strcpy(array_args, "./A");
            argv[0] = array_args;

            //Redirects pipes
            dup2(fd[A * 2 + READ], STDIN_FILENO);

            //Close pipes
            close(fd[A * 2 + WRITE]);
            close(fd[A * 2 + READ]);

            //Change code with A
            if (execvp(argv[0], argv) == -1) {
                value_return = err_exec(errno);
            }
        }
    }

    if (value_return == 0) {
        if (id == R && f == 0) {  //R SIDE
            strcpy(array_args, "./R");
            argv[0] = array_args;
            argv[1] = NULL;
            dup2(fd[R * 2 + READ], STDIN_FILENO);

            close(fd[R * 2 + READ]);
            close(fd[R * 2 + WRITE]);

            if (execvp(argv[0], argv) == -1) {
                value_return = err_exec(errno);
            }
        }
    }

    freeList(p);

    return value_return;
}

/*comandi possibili:
setn int setm int 	[da gestire]
info			
info -c //clustered info
close
help			[da modificare
*/

//Codici Univoci
//COdice per identificare un comando che deve essere mandato ad A. Per il momento io uso '2'
int check_command(char *cmd) {
    int res = -1;  //errore input comando
    if (strstr(cmd, "help") != NULL) {
        //mostra help comandi
        printf("Ci sarà una funzione help comandi\n");
        res = 1;
    } else if (strstr(cmd, "close") != NULL) {
        res = 0;
    } else if (strstr(cmd, "-c") != NULL) {
        res = 1;
    } else {  //Provvisorio per testare A in quanto manca il branch per i comandi da mandargli
        res = 2;
    }
    return res;
}
