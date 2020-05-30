#include "./lib/lib.h"

int check_command(char *str);  //controlla il comando un numero corrispondente: -1 comando non riconosciuto ecc

process *p;  //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)

void handle_sigint(int sig) {        //handler per il CTRL-C, ha l'obiettivo di
    int i = p->count - 1;            //Parto dalla fine (poiché nella lista i processi figli vengono salvati dopo il processo padre)
    if (i > 0) {                     //Se i > 0 => ci sono processi avviati
        while (i != 0) {             //Ciclo while fino a quando non ho controllato tutti i processi
            if (p->pid[i] > 0) {     //Controllo che non sia un processo padre
                kill(p->pid[i], 9);  //Provo a killare il pid[i]
            }
            i--;
        }
    }
    freeList(p);  //Libero la lista di processi che ho salvato
    exit(-1);     //Eseguo exit con codice di ritorno -1
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {  //Handler per ricezione di SIGTERM
    close_all_process();
}

void catch_sigterm() {  //handler per catturare il kill al di fuori del programma
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char *argv[]) {  //main
    catch_sigterm();

    signal(SIGINT, handle_sigint);  //Handler per SIGINT (Ctrl-C)

    p = create_process(1);  //Allocazione dinamica di p con dimensione 1
    int value_return = 0;   //Assegno il valore di default 0 a value return
    pid_t f = getpid();     //Inizializzo p e assegno il pid del processo M
    char cmd[DIM_CMD];      //contiente il comando lanciato
    char ch = '0';          //per leggere un carattere per volta
    int end = FALSE;        //TRUE se lanciato il comando "close"
    int res_cmd;
    int _write = TRUE;
    int i;
    int id;

    //IPC Variables--------------------------------------------------
    int fd[4];
    char array_args[4];

    if (argc == 1) {
        system("clear");
        printf(BOLDWHITE "BENVENUTO\n" RESET);
        printf("Usa " WHITE "help" RESET " per vedere l'elenco dei comandi\n");
        printf("Usa " WHITE "info" RESET " per avere informazioni riguardo al programma\n");
        printf("Usa " WHITE "clear" RESET " per pulire il terminale\n");
        printf("Usa " WHITE "close" RESET " per chiudere il programma\n");
        printf("Premi " WHITE "invio" RESET " per avviare il programma\n\n> ");
        fflush(stdout);
        do {
            strcpy(cmd, "");  //svuota la stringa per il prossimo comando
            fflush(stdout);
            ch = '\0';

            while (ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
                ch = getc(stdin);
                if (ch != '\n') strcat(cmd, &ch);  //concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
            }

            if (!strcmp(cmd, "info")) {  //Se il comando e` info
                printInfo();
            } else if (!strcmp(cmd, "help")) {  //Help comandi
                printHelp();
            } else if (!strcmp(cmd, "close")) {
                end = TRUE;
            } else if (!strcmp(cmd, "clear")) {
                system("clear");
                printf(BOLDWHITE "BENVENUTO\n" RESET);
                printf("Usa " WHITE "help" RESET " per vedere l'elenco dei comandi\n");
                printf("Usa " WHITE "info" RESET " per avere informazioni riguardo al programma\n");
                printf("Usa " WHITE "clear" RESET " per pulire il terminale\n");
                printf("Usa " WHITE "close" RESET " per chiudere il programma\n");
                printf("Premi " WHITE "invio" RESET " per avviare il programma\n\n> ");
                fflush(stdout);
            } else if(strcmp(cmd, "")){
                printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                fflush(stdout);
            }

        } while (strcmp(cmd, "") && strncmp(cmd, "close", 5));
    }

    //IPC
    if (value_return == 0 && !end) {        //Se il value return rimane a zero (=> non ci sono errori) prosegui
        for (i = 0; i < 3; i += 2) {        //Cicla
            if (pipe(fd + i) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                value_return = err_pipe();  //In caso di errore setta il valore di ritorno
            }
        }
    }

    //Set Non-blocking pipes (Shouldn't block anyway, just to be sure)
    if (value_return == 0 && !end) {      //Se il value return rimane a zero (=> non ci sono errori) prosegui
        if (unlock_pipes(fd, 4) == -1) {  //Se non riesci a sbloccare le pipe
            value_return = err_fcntl();   //Assegna a value return l'errore di fcntl
        }
    }
    insertProcess(p, getpid());                                  //Inserisci il processo con pid padre nella lista processi
    if (value_return == 0 && !end) {                             //Se il value return rimane a zero (=> non ci sono errori) prosegui
        for (i = 0; i < 2 && f > 0 && value_return == 0; i++) {  //Esegue il ciclo solo il padre e se non ci sono errori
            f = fork();                                          //esegui fork
            if (f == 0)
                id = i;  //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
            else if (f == -1)
                value_return = err_fork();  //vuol dire che c'è stato un errore, dunque value return prende il valore di errore nel fare forking
            else                            //altrimenti è andato a buon fine e siamo nel processo figlio:
                insertProcess(p, f);        //inserisci il nuovo processo con pid f nella lista processi avviati
        }
    }

    //--------------------------------------------------------------------------------
    if (value_return == 0) {
        if (f > 0) {                             //PARENT SIDE
            while (!end && value_return == 0) {  //CICLO DI ATTESA COMANDI IN INPUT
                strcpy(cmd, "");                 //svuota la stringa per il prossimo comando
                fflush(stdout);
                ch = '\0';

                while (ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
                    ch = getc(stdin);
                    if (ch != '\n') strcat(cmd, &ch);  //concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
                }

                res_cmd = check_command(cmd);  //Assegna il comando in base alla tipologia

                if (res_cmd == -1) {  //Comando inserito non corretto
                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                    fflush(stdout);
                }

                if (res_cmd == 0) {  //Se il comando e` uguale a close
                    end = TRUE;      //termina il ciclo
                    printf("\n");
                    while (value_return == 0 && _write) {                    //Manda il comando close ad A
                        if (write(fd[A * 2 + WRITE], cmd, DIM_CMD) == -1) {  //Controllo se ci sono errori
                            if (errno != EAGAIN) {                           //Se l'errore non è quello di EAGAIN
                                value_return = err_write();                  //A value return assegno l'errore nel write
                            }
                        } else {             //Altrimenti (non ci sono stati errori)
                            _write = FALSE;  //Smette di scrivere ad A
                        }
                    }
                    _write = TRUE;
                    while (value_return == 0 && _write) {  //Ripeto il ciclo come prima ma questa volta comunicando ad R la chiusura
                        if (write(fd[R * 2 + WRITE], cmd, DIM_CMD) == -1) {
                            if (errno != EAGAIN) {
                                value_return = err_write();
                            }
                        } else {
                            _write = FALSE;
                        }
                    }
                    _write = TRUE;
                    while (wait(NULL) > 0)  //Attesa della chiusura dei figli
                        ;
                }

                if (res_cmd == 1) {                                                       //Il comando deve essere mandato ad R
                    if (strstr(cmd, "print") != NULL || strstr(cmd, "report") != NULL) {  //Controllo se la stringa di comando è "print" o se contiene report
                        while (value_return == 0 && _write) {                             //Scrive il comando ad R
                            if (write(fd[R * 2 + WRITE], cmd, DIM_CMD) == -1) {           //Verifico la corretta scrittura su R, in caso negativo controllo:
                                if (errno != EAGAIN) {                                    //Se l'errore è differente da EAGAIN
                                    value_return = err_write();                           //Allora a value return assegno errore della write
                                }
                            } else {
                                _write = FALSE;  //Smette di scrivere ad R
                            }
                        }
                        _write = TRUE;  //Reimposta la write a TRUE
                    }
                }

                if (res_cmd == 2) {                                          //Il comando deve essere inviato ad A
                    while (value_return == 0 && _write) {                    //Scrive il comando ad A
                        if (write(fd[A * 2 + WRITE], cmd, DIM_CMD) == -1) {  //Verifico la corretta scrittura su A, in caso negativo controllo:
                            if (errno != EAGAIN) {                           //Se l'errore è differente da EAGAIN
                                value_return = err_write();                  //Allora a value return assegno errore della write
                            }
                        } else {
                            _write = FALSE;  //Smette di scrivere
                        }
                    }
                    _write = TRUE;  //Reimposta la write a TRUE
                }

                if (res_cmd == 3) {              //Il comando spetta ad M
                    if (!strcmp(cmd, "info")) {  //Se il comando e` info
                        printInfo();
                    } else {  //Help comandi
                        printHelp();
                    }
                }
            }
            close(fd[R * 2 + WRITE]);                        //Chiude la pipe di scrittura su R
            close(fd[R * 2 + READ]);                         //Chiude la pipe di lettura su R
            close(fd[A * 2 + WRITE]);                        //Chiude la pipe di scrittura su A
            close(fd[A * 2 + READ]);                         //Chiude la pipe di lettura su A
            printf(BOLDWHITE "M" RESET ": Closing...\n\n");  //Stampa la corretta chiusura
        }
    }

    if (value_return != 0) {
        close_all_process('M');
    }

    if (value_return == 0 && !end) {
        if (id == A && f == 0) {  //A SIDE

            strcpy(array_args, "./A");  //Copio ./A nella stringa array_args
            argv[0] = array_args;       //argv[0] assume valore: array_string

            //Redirects pipes
            dup2(fd[A * 2 + READ], STDIN_FILENO);  //Uso la pipe di A in read per il file descriptor

            //Close pipes
            close(fd[A * 2 + WRITE]);  //chiudo la pipe di scrittura con A
            close(fd[A * 2 + READ]);   //chiudo la pipe di lettura con A

            //Change code with A
            if (execvp(argv[0], argv) == -1) {   //Se l'avvio di argv da errore
                value_return = err_exec(errno);  //allora a value return assegno l'errore di esecuzione
            }
        }
    }

    if (value_return == 0 && !end) {
        if (id == R && f == 0) {                   //R SIDE
            strcpy(array_args, "./R");             //Copio "./R" nella stringa di array_args
            argv[0] = array_args;                  //ad argv[0] assegno array_args
            argv[1] = NULL;                        //ad argv[1] assegno NULL poiché non servono parametri aggiuntivi
            dup2(fd[R * 2 + READ], STDIN_FILENO);  //Uso la pipe di R in read per il file descriptor

            close(fd[R * 2 + READ]);   //chiudo la pipe di lettura con R
            close(fd[R * 2 + WRITE]);  //chiudo la pipe di scrittura con R

            if (execvp(argv[0], argv) == -1) {   //Se l'avvio di argv da erroe
                value_return = err_exec(errno);  //allora a value return assegno l'errore di esecuzione
            }
        }
    }

    freeList(p);  //libera la lista di processi

    return value_return;
}

int check_command(char *cmd) {
    int res = -1;  //Default comando sbagliato
    int spaces;

    if (!strcmp(cmd, "close")) {  //Se il comando e` close
        res = 0;
    } else if (!strncmp(cmd, "report", 6) || !strncmp(cmd, "print", 5)) {  //Se il comando contiene report oppure e` uguale a print
        res = 1;
        if (!strncmp(cmd, "report", 6)) {
            checkArg(cmd, &spaces);
            if (spaces != 2) res = -1;  //Controlla la corretta sintassi del comando
        }
    } else if (!strncmp(cmd, "add", 3) || !strncmp(cmd, "remove", 6) || !strcmp(cmd, "reset") || !strcmp(cmd, "analyze") || !strcmp(cmd, "reanalyze") || !strncmp(cmd, "set", 3) || !strncmp(cmd, "clear", 5) || !strncmp(cmd, "stat", 4) || !strncmp(cmd, "proc", 4)) {  //A
        res = 2;
    } else if (!strcmp(cmd, "help") || !strcmp(cmd, "info")) {  //HELP e info
        res = 3;
    }

    return res;
}
