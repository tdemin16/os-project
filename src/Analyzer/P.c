#include "../lib/lib.h"
int value_return = 0;

void sig_term_handler(int signum, siginfo_t* info, void* ptr) {
    value_return = err_kill_process_P();
}

void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char* argv[]) {
    catch_sigterm();
    //Argument passed
    int m = 4;
    int i;
    char path[DIM_PATH];
    char resp[DIM_RESP];

    //IPC Variables
    int* fd;
    int size_pipe;
    int f = getpid();
    int id;  //Identifica il numero del figlio generato
    int _read = FALSE;
    int _write = FALSE;
    int _close = FALSE;
    char sentClose = FALSE;
    array* sum = createPathList(10);
    char send_w = TRUE;
    char send_r = TRUE;
    int terminated[m];
    for (i = 0; i < m; i++) {
        terminated[i] = FALSE;
    }
    int sum_value;
    int oldfl;
    int pendingPath = 0;

    //Parsing arguments-------------------------------------------------------
    if (argc != 2) {
        value_return = err_args_P();
    } else {
        m = atoi(argv[1]);
        if (m == 0) value_return = err_m_not_valid();
    }

    //Generating pipes-------------------------------------------------------
    if (value_return == 0) {
        //Crea m*4 pipes (4 per coppia padre figlio, 2 in lettura e 2 in scrittura)
        size_pipe = m * 4;
        fd = (int*)malloc(size_pipe * sizeof(int));
        //Alloco le pipes a due a due
        for (i = 0; i < size_pipe - 1; i += 2) {
            if (pipe(fd + i) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                value_return = err_pipe();  //In caso di errore setta il valore di ritorno
            }
        }
    }
    /*  PIPES ENCODING---------------------------------------------
        fd[id*4 + 0] PARENT READ
        fd[id*4 + 1] SON WRITE
        fd[id*4 + 2] SON READ
        fd[id*4 + 3] PARENT WRITE
    */
    //-----------------------------------------------------------

    if (value_return == 0) {
        if (unlock_pipes(fd, size_pipe) == -1) {  //Set unblocking pipes
            value_return = err_fcntl();
        }
    }

    //Forking-----------------------------------------------------------
    if (value_return == 0) {
        //Ciclo m volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi m processi
        forkP(&m, &f, &id, &value_return);
    }

    //----------------------------------------------------------------------
    int checkRecived = 0;
    int son = 0;
    if (value_return == 0) {
        if (f > 0) {  //PARENT SIDE
            char str[15];
            sprintf(str, "P%d.txt", getpid());
            FILE* debug = fopen(str, "a");
            fprintf(debug, "AVVIATO P con m = %d\n", m);
            fclose(debug);
            while (checkRecived < m) {
                for (son = 0; son < m; son++) {
                    if (read(fd[son * 4 + 0], resp, DIM_RESP) > 0) {
                        if (!strncmp(resp, "#CHECK", 6)) {
                            checkRecived++;
                        }
                    }
                }
            }
            checkRecived = 0;
            debug = fopen(str, "a");
            fprintf(debug, "P: TUTTI I FIGLI GENERATI\n");
            fclose(debug);  //Setto end a true
            while (value_return == 0 && (!_close)) {
                //Write
                if (!_write) {                                         //Se non ha finito di scrivere
                    if (send_w) {                                      // se il file è stato mandato a tutti i q, leggo il prossimo
                        if (read(STDIN_FILENO, path, DIM_PATH) > 0) {  //provo a leggere
                            pendingPath++;
                            if (pendingPath == 1) {
                                if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
                                    value_return = err_fcntl();
                                }
                            }
                            debug = fopen(str, "a");
                            fprintf(debug, "P: LEGGO %s PENDING: %d\n", path, pendingPath);
                            fclose(debug);
                            if (!strncmp(path, "#CLOSE", 6)) {  //Se leggo una stringa di terminazione
                                _close = TRUE;
                                debug = fopen(str, "a");
                                fprintf(debug, "P: MI KILLO\n");
                                fclose(debug);  //Setto end a true
                            } else if (!strncmp(path, "#SETM#", 6)) {
                                debug = fopen(str, "a");
                                fprintf(debug, "P: NUOVO M: %d\n", m);
                                fclose(debug);  //Setto end a true
                                nClearAndClose(fd, m);
                                while (wait(NULL) > 0)
                                    ;
                                mParseOnFly(path, &m);
                                for (i = 0; i < size_pipe - 1; i += 2) {
                                    if (close(fd[i]) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                                        value_return = err_pipe();  //In caso di errore setta il valore di ritorno
                                    }
                                }
                                size_pipe = m * 4;
                                free(fd);
                                fd = (int*)malloc(size_pipe * sizeof(int));
                                //Alloco le pipes a due a due

                                for (i = 0; i < size_pipe - 1; i += 2) {
                                    if (pipe(fd + i) == -1) {       //Controlla se ci sono errori nella creazione della pipe
                                        value_return = err_pipe();  //In caso di errore setta il valore di ritorno
                                    }
                                }
                                if (unlock_pipes(fd, size_pipe) == -1) {  //Set nonblocking pipes
                                    value_return = err_fcntl();           //Gestione errore sullo sblocco pipe
                                }
                                if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {  //Sblocca lo stdin (teoricamente non necessario)
                                    value_return = err_fcntl();                  //Gestione errore sullo sblocco pipe
                                }
                                forkP(&m, &f, &id, &value_return);
                                if (f == 0) execP(&m, &f, &id, fd, &value_return, &size_pipe);
                                while (checkRecived < m) {
                                    for (son = 0; son < m; son++) {
                                        if (read(fd[son * 4 + 0], resp, DIM_RESP) > 0) {
                                            if (!strncmp(resp, "#CHECK", 6)) {
                                                checkRecived++;
                                                debug = fopen(str, "a");
                                                fprintf(debug, "P: RICEVUTO %d su %d\n", checkRecived, m);
                                                fclose(debug);
                                            }
                                        }
                                    }
                                }
                                
                                debug = fopen(str, "a");
                                fprintf(debug, "P: TUTTI I FIGLI GENERATI\n");
                                fclose(debug);
                                resetPathList(sum);
                                debug = fopen(str, "a");
                                fprintf(debug, "P: SUM LIST RESETTATA\n");
                                fclose(debug);
                                checkRecived = 0;
                                send_r = TRUE;
                                pendingPath--;
                            } else {
                                for (i = 0; i < m; i++) {  //Provo a inviare path a tutti i Q
                                    if (write(fd[i * 4 + 3], path, DIM_PATH) == -1) {
                                        if (errno != EAGAIN) {
                                            value_return = err_write();
                                        } else {
                                            send_w = FALSE;  //Se non ci riesce setta send a false
                                            terminated[i] = FALSE;
                                        }
                                    } else {
                                        terminated[i] = TRUE;
                                        debug = fopen(str, "a");
                                        fprintf(debug, "P: INVIATO %s\n", path);
                                        fclose(debug);
                                    }
                                }
                            }
                        }
                    } else {
                        send_w = TRUE;
                        for (i = 0; i < m; i++) {
                            if (!terminated[i]) {  //Per ogni path non inviato, riprova
                                if (write(fd[i * 4 + 3], path, DIM_PATH) == -1) {
                                    if (errno != EAGAIN) {
                                        value_return = err_write();
                                    } else {
                                        send_w = FALSE;  //send rimane TRUE se durante l'invio non ci sono stati problemi
                                    }
                                } else {
                                    terminated[i] = TRUE;
                                    debug = fopen(str, "a");
                                    fprintf(debug, "P: INVIATO %s\n", path);
                                    fclose(debug);
                                }
                            }
                        }
                    }
                }
                //Read
                if (!_read) {
                    if (!strncmp(resp,"#CHECK",6)){
                        send_r = TRUE;
                    }
                    if (send_r) {
                        for (i = 0; i < m; i++) {  //Cicla tra tutti i figli
                            if (read(fd[i * 4 + 0], resp, DIM_RESP) > 0) {
                                debug = fopen(str, "a");
                                fprintf(debug, "P: (<) LEGGO  %s\n", resp);
                                fclose(debug);
                                if (strstr(resp, "#") != NULL) {
                                    sum_value = insertAndSumPathList(sum, resp, m - 1);
                                    if (sum_value > -1) {  //Qualcosa è arrivato a 0,
                                        strcpy(resp, sum->pathList[sum_value]);
                                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
                                            if (errno != EAGAIN) {
                                                value_return = err_write();
                                            } else {
                                                send_r = FALSE;
                                                debug = fopen(str, "a");
                                                fprintf(debug, "P: Setto send_r a false\n");
                                                fclose(debug);
                                            }
                                        } else {
                                            pendingPath--;
                                            debug = fopen(str, "a");
                                            fprintf(debug, "P: RITORNO %s PENDING: %d\n", resp, pendingPath);
                                            fclose(debug);
                                        }
                                    }
                                }
                            }
                        }
                    } else {                                               //resend
                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
                            if (errno != EAGAIN) value_return = err_write();
                        } else {
                            send_r = TRUE;
                            pendingPath--;
                            debug = fopen(str, "a");
                            fprintf(debug, "P:s RITORNO %s PENDING: %d\n", resp, pendingPath);
                            fclose(debug);
                        }
                    }
                }
                if (pendingPath == 0) {
                    oldfl = fcntl(STDIN_FILENO, F_GETFL);
                    if (oldfl == -1) {
                        /* handle error */
                    }
                    fcntl(STDIN_FILENO, F_SETFL, oldfl & ~O_NONBLOCK);
                    resetPathList(sum);
                }
            }

            for (i = 0; i < m; i++) {
                while (read(fd[i * 4 + 3], path, DIM_PATH) > 0) {
                }
                terminated[i] = FALSE;
            }
            sentClose = FALSE;
            strcpy(path, "#CLOSE");
            while (!sentClose) {
                sentClose = TRUE;
                for (i = 0; i < m; i++) {  //Provo a inviare path a tutti i Q
                    if (write(fd[i * 4 + 3], path, DIM_PATH) == -1) {
                        if (errno != EAGAIN) {
                            value_return = err_write();
                        } else {
                            sentClose = FALSE;  //Se non ci riesce setta send a false
                            terminated[i] = FALSE;
                        }
                    } else {
                        terminated[i] = TRUE;
                    }
                }
            }

            while (wait(NULL) > 0)
                ;
            close_pipes(fd, size_pipe);
            free(fd);
            freePathList(sum);
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            execP(&m, &f, &id, fd, &value_return, &size_pipe);
        }
    }

    return value_return;
}