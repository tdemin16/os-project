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

int main(int argc, char *argv[]) {  //main
    signal(SIGINT, handle_sigint);  //Handler per SIGINT (Ctrl-C)

    //------------------------------ DEFINIZIONE VARIABILI
    p = create_process(1);  //Allocazione dinamica di p con dimensione 1
    int value_return = 0;   //Assegno il valore di default 0 a value return
    pid_t f = getpid();     //Inizializzo p e assegno il pid del processo M
    char cmd[DIM_CMD];      //contiente il comando lanciato
    char ch = '0';          //per leggere un carattere per volta
    int end = FALSE;        //TRUE se lanciato il comando "close"
    int res_cmd;            //Inizializzo variabile
    int _write = TRUE;      //Inizializzo variabile per il write e la assegno a TRUE
    int i;                  //Inizializzo variabile di iterazione i
    int id;                 //Inizializzo variabile
    int fptr;               //Inizializzo variabile
    int c[1];               //Inizializzo variabile

    //IPC Variables--------------------------------------------------
    int fd[4];           //Inizializzo variabile
    char array_args[4];  //Inizializzo variabile

    //IPC
    if (value_return == 0) {                //Se il value return rimane a zero (=> non ci sono errori) prosegui
        for (i = 0; i < 3; i += 2) {        //Cicla
            if (pipe(fd + i) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                value_return = err_pipe();  //In caso di errore setta il valore di ritorno
            }                               //
        }                                   //
    }                                       //

    //Set Non-blocking pipes (Shouldn't block anyway, just to be sure)
    if (value_return == 0) {                                     //Se il value return rimane a zero (=> non ci sono errori) prosegui
        if (unlock_pipes(fd, 4) == -1) {                         //Se non riesci a sbloccare le pipe
            value_return = err_fcntl();                          //Assegna a value return l'errore di fcntl
        }                                                        //
    }                                                            //
    insertProcess(p, getpid());                                  //Inserisci il processo con pid padre nella lista processi
    if (value_return == 0) {                                     //Se il value return rimane a zero (=> non ci sono errori) prosegui
        for (i = 0; i < 2 && f > 0 && value_return == 0; i++) {  //Cicla
            f = fork();                                          //esegui fork
            if (f == 0)                                          //se f == 0
                id = i;                                          //vuol dire che siamo ancora nel processo padre e quindi ad id assegna i
            else if (f == -1)                                    //se f == -1
                value_return = err_fork();                       //vuol dire che c'è stato un errore, dunque value return prende il valore di errore nel fare forking
            else                                                 //altrimenti è andato a buon fine e siamo nel processo figlio:
                insertProcess(p, f);                             //inserisci il nuovo processo con pid f nella lista processi avviati
        }                                                        //
    }                                                            //

    //--------------------------------------------------------------------------------
    if (value_return == 0) {                     //Se il value return rimane a zero (=> non ci sono errori) prosegui
        if (f > 0) {                             //PARENT SIDE
            while (!end && value_return == 0) {  //--------- CICLO DI ATTESA COMANDI IN INPUT
                strcpy(cmd, "");                 //svuota la stringa per il prossimo comando
                fflush(stdout);                  //
                ch = '\0';                       //

                while (ch != '\n') {                   //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
                    ch = getc(stdin);                  //
                    if (ch != '\n') strcat(cmd, &ch);  //concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
                }                                      //

                res_cmd = check_command(cmd);  //

                if (res_cmd == -1) {                                                                                                                    //richiama la funzione help() coi comandi, se il valore è -1 vuol dire che il comando inserito non è corretto
                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");  //stampo l'errore
                    fflush(stdout);                                                                                                                     //libero il buffer
                }                                                                                                                                       //

                if (res_cmd == 0) {                                          //Da qui in poi il comando è valido, se res_cmd == 0 allora invochiamo una close
                    end = TRUE;                                              //flag per comunicare la chiusura ai processi figli
                    printf("\n");                                            //
                    while (value_return == 0 && _write) {                    //Finché posso scrivere e finché il valore di ritorno è uguale a zero mando ad A il flag end
                        if (write(fd[A * 2 + WRITE], cmd, DIM_CMD) == -1) {  //Controllo se ci sono errori
                            if (errno != EAGAIN) {                           //Se l'errore non è quello di EAGAIN
                                value_return = err_write();                  //A value return assegno l'errore nel write
                            }                                                //
                        } else {                                             //Altrimenti (non ci sono stati errori)
                            _write = FALSE;                                  //La write va a false
                        }                                                    //
                    }                                                        //
                    _write = TRUE;                                           //
                    while (value_return == 0 && _write) {                    //Ripeto il ciclo come prima ma questa volta comunicando ad R il flag end
                        if (write(fd[R * 2 + WRITE], cmd, DIM_CMD) == -1) {  //
                            if (errno != EAGAIN) {                           //
                                value_return = err_write();                  //
                            }                                                //
                        } else {                                             //
                            _write = FALSE;                                  //
                        }                                                    //
                    }                                                        //
                    _write = TRUE;                                           //Reimposto la write a TRUE
                    while (wait(NULL) > 0)
                        ;  // ---??? Non comprendo il perché del ciclo ???---
                }          //

                if (res_cmd == 1) {                                              //Se invece res_cmd == 1 dobbiamo stabilire una connessione tra il processo M ed R
                    if (!strcmp(cmd, "print") || strstr(cmd, "report")) {        //Controllo se la stringa di comando è "print" o se contiene report
                        while (value_return == 0 && _write) {                    //Fino a che non ho errori e posso scrivere
                            if (write(fd[R * 2 + WRITE], cmd, DIM_CMD) == -1) {  //Verifico la corretta scrittura su R, in caso negativo controllo:
                                if (errno != EAGAIN) {                           //Se l'errore è differente da EAGAIN
                                    value_return = err_write();                  //Allora a value return assegno errore della write
                                }                                                //
                            } else {                                             //Altrimenti
                                _write = FALSE;                                  //La write va a false
                            }                                                    //
                        }                                                        //
                        _write = TRUE;                                           //Reimposto la write a TRUE
                    }                                                            //
                }                                                                //

                if (res_cmd == 2) {                                          //Se invece res_cmd == 2 allora dobbiamo stabilire una connessione tra il processo M ed A
                    while (value_return == 0 && _write) {                    //Fino a che non ho errori e posso scrivere
                        if (write(fd[A * 2 + WRITE], cmd, DIM_CMD) == -1) {  //Verifico la corretta scrittura su A, in caso negativo controllo:
                            if (errno != EAGAIN) {                           //Se l'errore è differente da EAGAIN
                                value_return = err_write();                  //Allora a value return assegno errore della write
                            }                                                //
                        } else {                                             //Altrimenti
                            _write = FALSE;                                  //La write va a false
                        }                                                    //
                    }                                                        //
                    _write = TRUE;                                           //Reimposto la write a TRUE
                }                                                            //

                if (res_cmd == 3) {                                  //Se infine res_cmd == 3 allora dobbiamo stampare le info o l'help
                    if (!strcmp(cmd, "info")) {                      //Nel caso in cui l'utente abbia inserito "info" allora dobbiamo visualizzare il readme:
                        fptr = open("../README.txt", O_RDONLY);      //Apro il file di README in sola lettura
                        printf("\n");                                //Vado a capo
                        if (fptr != -1) {                            //Se non ci sono stati errori
                            while (read(fptr, c, 1)) putchar(c[0]);  //Finché puoi leggere dal file stampa i caratteri (fino ad ottenere l'intero file)
                            close(fptr);                             //Infine chiudi il file
                        } else {                                     //Altrimenti c'è stato un errore nell'apertura del file
                            err_file_open();
                        }
                        printf(BOLDWHITE"\nCRITERI DI CLUSTERING\n"RESET);
                        printInfoCluster();
                        printf("\n> ");  //A capo
                        fflush(stdout);  //Libero il buffer
                    } else {
                        printf(BOLDBLUE "\nLista Comandi disponibili\n" RESET);                                                                      //STAMPA INFO
                        printf(BOLDWHITE "add </path1> </path2>" RESET ": aggiunge uno o piu` file e/o una o piu` directory\n");                     //
                        printf(BOLDBLACK "\t es: add ../src ../deploy.sh\n" RESET);                                                                  //
                        printf(BOLDWHITE "remove </path1> </path2>" RESET ": rimuove uno o piu` file e/o una o piu` directory\n");                   //
                        printf(BOLDBLACK "\t es: remove ../src ../deploy.sh\n" RESET);                                                               //
                        printf(BOLDWHITE "reset" RESET ": elimina dalla cache del programma le statistiche e tutti i percorsi analizzati e non\n");  //
                        printf(BOLDWHITE "print" RESET ": stampa a video tutte il percorso di tutti i file analizzati\n");                           //
                        printf(BOLDWHITE "analyze" RESET ": avvia l'analizzatore\n");                                                                //
                        printf(BOLDWHITE "set <n> <m>" RESET ": setta i nuovi valori di n e m\n");                                                   //
                        printf(BOLDBLACK "\t es: set 4 5\n" RESET);                                                                                  //
                        printf(BOLDWHITE "setn <val>" RESET ": setta i nuovi valori di n\n");                                                        //
                        printf(BOLDBLACK "\t es: setn 4\n" RESET);                                                                                   //
                        printf(BOLDWHITE "setm <val>" RESET ": setta i nuovi valori di m\n");                                                        //
                        printf(BOLDBLACK "\t es: setm 5\n" RESET);                                                                                   //
                        printf(BOLDWHITE "report" RESET " <-flag>\n");                                                                               //
                        printf(WHITE "\t-c" RESET ": stampa le statistiche per cluster\n");                                                          //
                        printf(WHITE "\t-a" RESET ": stampa la frequenza di ogni carattere\n");                                                      //
                        printf(BOLDWHITE "info" RESET ": mostra informazioni aggiuntive sul programma\n");                                           //
                        printf(BOLDWHITE "close" RESET ": chiude il programma\n\n> ");                                                               //
                        fflush(stdout);                                                                                                              //
                    }                                                                                                                                //
                }                                                                                                                                    //
            }
            close(fd[R * 2 + WRITE]);                        //Chiude la pipe di scrittura su R
            close(fd[R * 2 + READ]);                         //Chiude la pipe di lettura su R
            close(fd[A * 2 + WRITE]);                        //Chiude la pipe di scrittura su A
            close(fd[A * 2 + READ]);                         //Chiude la pipe di lettura su A
            printf(BOLDWHITE "M" RESET ": Closing...\n\n");  //Stampa la corretta chiusura
        }                                                    //
    }                                                        //

    if (value_return == 0) {      //Se il value return rimane a zero (=> non ci sono errori) prosegui
        if (id == A && f == 0) {  //A SIDE

            //Change binary file
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

    if (value_return == 0) {                       //Se il value return rimane a zero (=> non ci sono errori) prosegui
        if (id == R && f == 0) {                   //R SIDE
            strcpy(array_args, "./R");             //Copio "./R" nella stringa di array_args
            argv[0] = array_args;                  //ad argv[0] assegno array_args
            argv[1] = NULL;                        //ad argv[1] assegno NULL poiché non servono parametri aggiuntivi
            dup2(fd[R * 2 + READ], STDIN_FILENO);  //Uso la pipe di R in read per il file descriptor
                                                   //
            close(fd[R * 2 + READ]);               //chiudo la pipe di lettura con R
            close(fd[R * 2 + WRITE]);              //chiudo la pipe di scrittura con R
                                                   //
            if (execvp(argv[0], argv) == -1) {     //Se l'avvio di argv da erroe
                value_return = err_exec(errno);    //allora a value return assegno l'errore di esecuzione
            }                                      //
        }                                          //
    }                                              //
                                                   //
    freeList(p);                                   //libero la lista di processi
                                                   //
    return value_return;                           //ritorno value_return
}  //

//Codici Univoci
//Codice per identificare un comando che deve essere mandato ad A. Per il momento io uso '2'
int check_command(char *cmd) {  //
    int res = -1;               //errore input comando
    int spaces;                 //

    if (!strcmp(cmd, "close")) {                                          //CLOSE
        res = 0;                                                          //
    } else if (strstr(cmd, "report") != NULL || !strcmp(cmd, "print")) {  //R
        res = 1;                                                          //
        if (!strncmp(cmd, "report", 6)) {                                 //
            checkArg(cmd, &spaces);                                       //
            if (spaces != 2) res = -1;                                    //
        }
    } else if (!strncmp(cmd, "add", 3) || !strncmp(cmd, "remove", 6) || !strcmp(cmd, "reset") || !strcmp(cmd, "analyze") || !strcmp(cmd, "reanalyze") || !strncmp(cmd, "set", 3) || !strncmp(cmd, "clear", 4) || !strncmp(cmd, "oldprint", 8) || !strncmp(cmd, "debug", 5)) {  //A
        res = 2;
    } else if (!strcmp(cmd, "help") || !strcmp(cmd, "info")) {  //HELP
        res = 3;
    }

    return res;
}
