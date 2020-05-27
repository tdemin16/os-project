#include "../lib/lib.h"

int value_return = 0;  //Valore di ritorno, globale per "send_to_R"
process *p;            //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)
int fd1_fifo;          //A writes in R
int fd2_fifo;          //R writes in A

void handle_sigint(int sig) {
    printf("\n[!] Ricevuta terminazione per A, inizio terminazione processi C,P,Q ... \n");
    int i = p->count - 1;  //start from the end
    if (i > 0) {
        while (i != 0)  //while we haven't controlled every single process
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
    }
    if (!close(fd1_fifo) && !close(fd2_fifo)) {  //close fifo
        printf("[!] Chiusura fifo completata\n");
    }

    freeList(p);  //free memory allocated for p
    printf("[!] ... Chiusura processi C,P,Q terminata\n");
    exit(-1);  //return exit with error -1
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    value_return = err_kill_process_A();
}

void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char *argv[]) {
    printf("Start A\n");
    catch_sigterm();
    signal(SIGINT, handle_sigint);  //Handler for SIGINT (Ctrl-C)

    p = create_process(1);  //Allocate dynamically p with dimension 1

    //COMMUNICATION WITH R
    const char *fifo1 = "/tmp/A_to_R";  //Nome fifo con R
    const char *fifo2 = "/tmp/R_to_A";
    int retrieve = TRUE;
    int p_create = FALSE;
    char print_method[DIM_CMD];
    char type_resp[DIM_RESP];
    char tmp_resp[DIM_PATH];
    strcpy(tmp_resp, "///");

    //COMMUNICATION WITH M - STDIN
    char cmd[DIM_CMD];  //Comando rivevuto da M
    int _close = FALSE;
    char *new_n;
    char *new_m;
    char *dupl = NULL;

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;

    //Utility
    int i;  //Variabile usata per ciclare gli argomenti (argv[i])
    int j;
    int count = 0;    //numero di file univoci da analizzare
    int perc = 0;     //Ricevimento parziale file
    int oldperc = 0;  //Parziale precedente
    char n_exec[12];
    char m_exec[12];

    char analyzing = FALSE;
    int pathSent = 0;
    char *tmp = NULL;
    char *tmpResp = NULL;
    char *tmpPercorso = NULL;
    char **tempPath;
    int vReturn;

    array *lista = createPathList(10);  //Nuova lista dei path

    //Variables for IPC
    int fd_1[2];  //Pipes
    int fd_2[2];
    pid_t f;              //fork return value
    int _write = TRUE;    //true when finish writing the pipe
    int _read = TRUE;     //true when fisnish reading from pipe
    char resp[DIM_RESP];  //Stringa in cui salvare i messaggi ottenuti dal figlio
    int id_r;             //Id file ricevuto
    char *resp_val;       //Messaggio senza Id
    char *file;           //Messaggio senza Id e identificatori (#)
    int firstVal = 0;     //Controllo sulla validita' di un messaggio
    char sum[DIM_RESP];
    int v[DIM_V];         //Array con valori totali
    int notAnalyzed = 0;  //Flag indicante se e` avvenuta o meno la lettura della pipe
    int argCounter = 0;
    initialize_vector(v);  //Inizializzazione vettore dei valori totali
    int val = 0;

    if (argc > 1) {
        val = parser2(argc, argv, lista, &count, &n, &m, &vReturn);
        if (val == 0) {  //Controlla i parametri passati ad A
            if (vReturn > 0) {
                _write = FALSE;
            }
        } else {
            system("clear");
            printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n");
            fflush(stdout);
            if (val == 1) err_args_A();
            printf("> ");
            fflush(stdout);
        }
    }

    insertProcess(p, getpid());  //Insert pid of A in process list

    //IPC
    if (value_return == 0) {            //Testo che non si siano verificati errori in precedenza
        if (pipe(fd_1) != 0) {          //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe();  //in caso di git errore setta il valore di ritorno a ERR_PIPE
        }
    }
    if (value_return == 0) {            //Testo che non si siano verificati errori in precedenza
        if (pipe(fd_2) != 0) {          //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe();  //in caso di errore setta il valore di ritorno a ERR_PIPE
        }
    }

    //Set Non-blocking pipes
    if (value_return == 0) {
        if (fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare la pipe 1 in lettura
            value_return = err_fcntl();                //Se errore riporta il messaggio di errore
        }
        if (fcntl(fd_1[WRITE], F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare la pipe 1 in scrittura
            value_return = err_fcntl();                 //Se errore riporta il messaggio di errore
        }
        if (fcntl(fd_2[READ], F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare la pipe 2 in lettura
            value_return = err_fcntl();                //Se errore riporta il messaggio di errore
        }
        if (fcntl(fd_2[WRITE], F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare la pipe 2 in scrittura
            value_return = err_fcntl();                 //Se errore riporta il messaggio di errore
        }
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare lo stdin
            value_return = err_fcntl();
        }
    }

    //Open fifo in nonblocking read mode
    if (value_return == 0) {
        printf("Waiting for R...\n");
        printf("Use " BOLDYELLOW "[CTRL+C]" RESET " to interrupt\n");
        if (mkfifo(fifo1, 0666) == -1) {    //Prova a creare la pipe
            if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
            }
        }
        fd1_fifo = open(fifo1, O_WRONLY);        //Prova ad aprire la pipe in scrittura
        if (fd1_fifo == -1) {                    //Error handling
            if (errno != ENOENT) {               //Se errno == 6, il file A non e' stato ancora aperto
                value_return = err_file_open();  //Errore nell'apertura del file
            }
        }
        printf("A OK\n");
        do {
            if (mkfifo(fifo2, 0666) == -1) {    //Prova a creare la pipe
                if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                    value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
                }
            }
            fd2_fifo = open(fifo2, O_RDONLY | O_NONBLOCK);
            if (fd2_fifo != -1) {
                p_create = TRUE;
            } else if (errno != ENOENT) {
                value_return = err_fifo();
            }
        } while (value_return == 0 && !p_create);

    }

    if (argc == 1) {
            system("clear");
            printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n\n");
            fflush(stdout);

            printf("> ");
            fflush(stdout);
        }

    if (value_return == 0 && !_close) {
        f = fork();                     //Fork dei processi
        if (f == -1) {                  //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork();  //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    //------------------------------------------------------------------------------

    if (value_return == 0 && !_close) {
        if (f > 0) {              //PARENT SIDE
            insertProcess(p, f);  //Insert child process in list p
            i = 0;
            char str[15];
            sprintf(str, "A%d.txt", getpid());
            FILE *debug = fopen(str, "a");
            fprintf(debug, "AVVIATO A\n");
            fclose(debug);

            while (value_return == 0 && !_close) {  //cicla finche` non ha finito di leggere e scrivere o avviene un errore

                //M - STDIN
                if (!_close) {
                    if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {
                        if (!strncmp(cmd, "close", 5)) {
                            closeAll(fd_1);

                            while (wait(NULL) > 0)
                                ;
                            _close = TRUE;
                            printf(BOLDWHITE "A" RESET ": Closing...\n");
                        }

                        if (!strncmp(cmd, "add", 3)) {
                            if (!analyzing) {
                                if ((strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL)) {
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");
                                    fflush(stdout);
                                } else if (checkArg(cmd, &argCounter)) {
                                    tempPath = malloc(argCounter * sizeof(char *));
                                    for (j = 0; j < argCounter; j++) {
                                        tempPath[j] = malloc(DIM_PATH * sizeof(char));
                                    }
                                    strcpy(tempPath[0], strtok(cmd, " "));
                                    for (j = 1; j < argCounter; j++) {
                                        strcpy(tempPath[j], strtok(NULL, " "));
                                    }
                                    if ((parser2(argCounter, tempPath, lista, &count, &n, &m, &vReturn)) == 0) {
                                        printf("Aggiunti %d files\n", vReturn);
                                    } else {
                                        //err_args_A();
                                    }
                                    for (j = 0; j < argCounter; j++) {
                                        free(tempPath[j]);
                                    }
                                    free(tempPath);
                                } else {
                                    err_args_A();
                                }
                            } else {
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "remove", 6)) {
                            if (!analyzing) {
                                if ((strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL)) {
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");
                                    fflush(stdout);
                                } else if (checkArg(cmd, &argCounter)) {
                                    tempPath = malloc(argCounter * sizeof(char *));
                                    for (j = 0; j < argCounter; j++) {
                                        tempPath[j] = malloc(DIM_PATH * sizeof(char));
                                    }
                                    strcpy(tempPath[0], strtok(cmd, " "));
                                    for (j = 1; j < argCounter; j++) {
                                        strcpy(tempPath[j], strtok(NULL, " "));
                                    }
                                    if ((parser2(argCounter, tempPath, lista, &count, &n, &m, &vReturn)) == 0) {
                                        cleanRemoved(lista);
                                        printf("Rimossi %d files\n", vReturn);
                                    } else {
                                        //err_args_A();
                                    }
                                    for (j = 0; j < argCounter; j++) {
                                        //printf("ARG[%d] - %s\n",j,tempPath[j]);
                                        free(tempPath[j]);
                                    }
                                    free(tempPath);
                                } else {
                                    err_args_A();
                                }
                            } else {
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "reset", 5)) {
                            if (!analyzing) {
                                resetPathList(lista);
                                count = 0;
                                memset(sum, '\0', sizeof(char) * DIM_RESP);
                                initialize_vector(v);
                            } else {
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "debug", 5)) {
                            printf("count = %d\npathSent=%d\n", count, pathSent);
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "reanalyze", 9)) {
                            if (!analyzing) {
                                for (j = 0; j < lista->count; j++) {
                                    lista->analyzed[j] = 0;
                                }
                                pathSent = 0;
                                notAnalyzed = 0;
                                perc = 0;
                                count = lista->count;
                                if (count > 0) {
                                    memset(sum, '\0', sizeof(char) * DIM_RESP);
                                    initialize_vector(v);
                                    _write = FALSE;
                                } else {
                                    printf("Non ci sono file da analizzare\n> ");
                                    fflush(stdout);
                                }
                            } else {
                                printf("Analisi in corso, comando non disponibile\n> ");
                                fflush(stdout);
                            }
                        }

                        if (!strncmp(cmd, "analyze", 7)) {
                            if (!analyzing) {
                                pathSent = 0;
                                notAnalyzed = 0;
                                perc = 0;
                                if (count > 0)
                                    _write = FALSE;
                                else
                                    printf("\nNon ci sono file da analizzare\n");
                            } else {
                                printf("\nAnalisi in corso, comando non disponibile\n");
                            }
                            printf("\n> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "oldprint", 8)) {
                            printPathList(lista);
                            printf("\n> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "clear", 5)) {
                            system("clear");
                            printf("\n> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "set", 3)) {
                            if (checkArg(cmd, &argCounter)) {
                                if (argCounter == 2) {
                                    if (!strncmp(cmd, "setn", 4)) {
                                        dupl = strdup(cmd);
                                        new_n = strtok(dupl, " ");
                                        new_n = strtok(NULL, " ");
                                        if (atoi(new_n) > 0) {
                                            n = atoi(new_n);
                                            setOnFly(n, m, fd_1);
                                        } else {
                                            printf("\nValore di n non valido\n");
                                        }

                                        free(dupl);

                                    } else if (!strncmp(cmd, "setm", 4)) {
                                        dupl = strdup(cmd);
                                        new_m = strtok(dupl, " ");
                                        new_m = strtok(NULL, " ");
                                        if (atoi(new_m) > 0) {
                                            m = atoi(new_m);
                                            setmOnFly(m, fd_1);
                                        } else {
                                            printf("\nValore di m non valido\n");
                                        }
                                        free(dupl);
                                    }
                                } else if (argCounter == 3) {
                                    parseSetOnFly(cmd, &n, &m);
                                    //printf("\nn:%d m:%d\n\n",n,m);
                                    setOnFly(n, m, fd_1);
                                }
                            }
                            printf("> ");
                            fflush(stdout);
                        }
                    }
                }

                //R
                if (!_close && value_return == 0) {
                    if (retrieve) {  //Try read from R
                        if (read(fd2_fifo, print_method, DIM_CMD) > 0) {
                            if (!strncmp(print_method, "print", 5) || !strncmp(print_method, "-c", 2)) {
                                retrieve = FALSE;
                            }
                        }
                    } else {
                        if (!strncmp(print_method, "print", 5)) {
                            if (lista->count > 0) {
                                for (j = 0; j < lista->count; j++) {
                                    write(fd1_fifo, lista->pathList[j], DIM_PATH + 2);
                                }
                            }

                            write(fd1_fifo, tmp_resp, DIM_PATH + 2);
                        }
                        if (!strncmp(print_method, "-c", 2)) {
                            analyzeCluster(sum, type_resp);
                            write(fd1_fifo, type_resp, DIM_RESP);
                        }
                        retrieve = TRUE;
                    }
                }

                //Quando WRITE e' in funzione inizia a mandare tutti i file con flag 0 di pathList
                if (!_write && value_return == 0) {  //Esegue il blocco finche` non ha finito di scrivere
                    analyzing = TRUE;
                    if (lista->analyzed[i] == 0) {
                        if (write(fd_1[WRITE], lista->pathList[i], DIM_PATH) == -1) {  //Prova a scrivere sulla pipe
                            if (errno != EAGAIN) {                                     //Se avviene un errore e non e` causato dalla dimensione della pipe
                                value_return = err_write();                            //Ritorna l'errore sulla scrittura
                            }
                        } else {
                            debug = fopen(str, "a");
                            fprintf(debug, "A: INVIATO %s\n", lista->pathList[i]);
                            fclose(debug);
                            i++;
                            pathSent++;
                            if (pathSent == 1) {
                                _read = FALSE;
                                debug = fopen(str, "a");
                                fprintf(debug, "A: ABILITO LA READ\n");
                                fclose(debug);
                            }
                        }
                    } else {
                        i++;
                    }
                    if (i == lista->count) {  //Qunado ha finito di inviare
                        _write = TRUE;        //Setta il flag a true
                        i = 0;
                        debug = fopen(str, "a");
                        fprintf(debug, "A: CHIUDO LA READ\n");
                        fclose(debug);
                    }
                }

                //Read
                if (!_read && value_return == 0) {               //Esegue il blocco fiche` non c'e` piu` nulla nella pipe
                    if (read(fd_2[READ], resp, DIM_RESP) > 0) {  //Pero` potremmo vedere se sto controllo serve realmente
                        if (strstr(resp, "#") != NULL) {         //Controlla che ci sia almeno un # nel messaggio
                            tmp = strdup(resp);
                            id_r = atoi(strtok(tmp, "#"));  //id del file da valutare
                            resp_val = strtok(NULL, "#");   //valori
                            tmpResp = strdup(resp_val);
                            firstVal = atoi(strtok(tmpResp, ","));  //primo valore
                            tmpPercorso = strdup(lista->pathList[id_r]);
                            file = strtok(tmpPercorso, "#");  //Recupera path corrispondente nella lista
                            file = strtok(NULL, "#");         //percorso
                            if (firstVal != -1) {
                                if (fileExist(file)) {                                              // File esistente
                                    lista->analyzed[id_r] = 1;                                      //Setta il flag ad Analizzato
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                    if (value_return == 0) {
                                        if (!compare_mtime(lista, id_r, file)) {
                                            printf("\nIl file %s\ne` stato modificato durante l'analisi.\nUsa il comando" BOLDWHITE " reanalyze" RESET " per rianalizzarlo\n\n", file);
                                        }
                                    }
                                } else {
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    lista->analyzed[id_r] = 2;                                      //Setta il flag ad analizzato ma non piu` esistente
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                }
                            } else {  //Caso in cui il file non e` piu' esistente
                                notAnalyzed++;
                                lista->analyzed[id_r] = -1;
                                perc++;
                            }

                            //Barretta

                            /* char *ptr = strchr(lista->pathList[id_r], '\0');
                            if (ptr) {
                                int index = ptr - lista->pathList[id_r];
                                char *last_ten = &lista->pathList[id_r][index - 50];
                                printf("\033[A\33[2KT\r"BOLDGREEN"[ANALYZED]"RESET" ..%s\n", last_ten);
                                fflush(stdout);
                            } */

                            if ((int)((float)perc * 10 / (float)pathSent) > oldperc && value_return == 0) {
                                oldperc = (int)((float)perc * 10 / (float)pathSent);
                                //system("clear");
                                //percAvanzamento(perc, count);
                            }

                            if (_write == TRUE && perc == pathSent && value_return == 0) {
                                count -= lista->count;
                                printf("Numero file analizzati: %d\n\n> ", pathSent);
                                fflush(stdout);
                                arrayToCsv(v, sum);
                                //printStat_Cluster(sum);
                                //printf("\n> ");
                                //fflush(stdout);
                                //setOnFly(4,5,fd_1);
                                //sleep(5);
                                //closeAll(fd_1);
                                //_close = TRUE;
                                pathSent = 0;
                                analyzing = FALSE;
                                _read = FALSE;
                            }

                            free(tmpPercorso);
                            free(tmp);
                            free(tmpResp);
                        }
                    }
                }
            }

            //Chiusura pipe
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);

            //Chiusura fifo
            if (value_return == 0) {
                if (close(fd1_fifo) == -1) {
                    value_return = err_close();
                }
                if (close(fd2_fifo) == -1) {
                    value_return = err_close();
                }
            }

            freePathList(lista);
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            sprintf(n_exec, "%d", n);
            sprintf(m_exec, "%d", m);
            char *args[4] = {"./C", n_exec, m_exec, NULL};
            //Redirects pipes to STDIN and STDOUT
            dup2(fd_1[READ], STDIN_FILENO);
            dup2(fd_2[WRITE], STDOUT_FILENO);
            //Closing pipes
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);
            if (execvp(args[0], args) == -1) {   //Test exec
                value_return = err_exec(errno);  //Set value return
            }
            /*
            //Creates char* args []
            strcpy(array[0], "./C");
            strcpy(array[1], "-nfiles");
            sprintf(array[2], "%d", count);
            strcpy(array[3], "-setn");
            sprintf(array[4], "%d", n);
            strcpy(array[5], "-setm");
            sprintf(array[6], "%d", m);
            //Copy into args
            for (i = 0; i < 7; i++) {
                args[i] = array[i];
            }
            args[7] = NULL;

            //Redirects pipes to STDIN and STDOUT
            dup2(fd_1[READ], STDIN_FILENO);
            dup2(fd_2[WRITE], STDOUT_FILENO);
            //Closing pipes
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);
            if (execvp(args[0], args) == -1) {   //Test exec
                value_return = err_exec(errno);  //Set value return
            }
            */
        }
    }

    freeList(p);

    return value_return;
}