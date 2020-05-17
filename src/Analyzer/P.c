//Version updated to 2020.05.02
//P si occupano di gestire ciascuno un sottoinsieme degli input (partizionato in “n” sottogruppi) creando a sua volta “m” figli
#include "../lib/lib.h"

int main(int argc, char* argv[]) {
    //Argument passed
    int m = 4;

    int value_return = 0;
    int i;
    char path[PATH_MAX];
    char failedPath[m][PATH_MAX];
    char resp[DIM_RESP];

    //IPC Variables
    int* fd;
    int size_pipe;
    int f = getpid();
    int id;  //Identifica il numero del figlio generato
    int _read = FALSE;
    int _write = FALSE;
    array* sum = createPathList(10);
    char array[4][4];
    char* args[4];
    int count = 0;
    char stop = FALSE;
    for (i = 0; i < m; i++) {
        failedPath[i][0] = '\0';
    }
    int cc = 0;
    int u;
    char send_w = TRUE;
    char send_r = TRUE;
    char end = FALSE;
    int terminated[m];
    int test = 0;
    for (i = 0; i < m; i++) {
        terminated[i] = FALSE;
    }
    int sum_value;

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
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }

    //Forking-----------------------------------------------------------
    if (value_return == 0) {
        //Ciclo m volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi m processi
        for (i = 0; i < m && f > 0 && value_return == 0; i++) {
            f = fork();
            if (f == 0) {  //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
                id = i;
            } else if (f == -1) {
                value_return = err_fork();
            } /*else{
                //insert_process(f);
            }*/
        }
    }

    //----------------------------------------------------------------------
    if (value_return == 0) {
        if (f > 0) {  //PARENT SIDE
            while (value_return == 0 && (!_read || !_write)) {
                //Write
                if (!_write) {                                         //Se non ha finito di scrivere
                    if (send_w) {                                      // se il file è stato mandato a tutti i q, leggo il prossimo
                        if (read(STDIN_FILENO, path, PATH_MAX) > 0) {  //provo a leggere
                            if (!strncmp(path, "///", 3)) {            //Se leggo una stringa di terminazione
                                end = TRUE;                            //Setto end a true
                                //fprintf(stderr,"C finito di scrivere, %s\n",path);
                            }
                            for (i = 0; i < m; i++) {  //Provo a inviare path a tutti i Q
                                if (write(fd[i * 4 + 3], path, PATH_MAX) == -1) {
                                    if (errno != EAGAIN) {
                                        value_return = err_write();
                                    } else {
                                        send_w = FALSE;  //Se non ci riesce setta send a false
                                        terminated[i] = FALSE;
                                    }
                                } else {
                                    terminated[i] = TRUE;
                                }
                            }
                        }
                    } else {
                        //fprintf(stderr,"C[%d] provo ritrasmissione di %s\n",getpid(),path);
                        send_w = TRUE;
                        for (i = 0; i < m; i++) {
                            if (!terminated[i]) {  //Per ogni path non inviato, riprova
                                if (write(fd[i * 4 + 3], path, PATH_MAX) == -1) {
                                    if (errno != EAGAIN) {
                                        value_return = err_write();
                                    } else {
                                        send_w = FALSE;  //send rimane TRUE se durante l'invio non ci sono stati problemi
                                    }
                                } else {
                                    terminated[i] = TRUE;
                                }
                            }
                        }
                    }
                    if (end && send_w) {  //Se lo stato e' end, e tutto e' stato inviato, allora la write e' finita
                        //fprintf(stderr,"C finito di scrivere, %s\n",path);
                        _write = TRUE;
                    }
                }

                //Read
                if (!_read) {
                    if (send_r) {
                        for (i = 0; i < m; i++) {  //Cicla tra tutti i figli
                            if (read(fd[i * 4 + 0], resp, DIM_RESP) > 0) {
                                //fprintf(stderr,"P read: %s\n",resp);
                                if (!strcmp(resp, "///")) {                                //Controlla se e` la fine del messaggio
                                    count++;                                               //Conta quanti terminatori sono arrivati
                                    if (count == m) {                                      //Quando tutti i figli hanno terminato
                                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
                                            if (errno != EAGAIN) {
                                                value_return = err_write();
                                            } else {
                                                send_r = FALSE;
                                            }
                                        }
                                    }
                                } else if (strstr(resp, "#") != NULL) {
                                    sum_value = insertAndSumPathList(sum, resp, m - 1);
                                    if (sum_value > -1) {  //Qualcosa è arrivato a 0,
                                        strcpy(resp, sum->pathList[sum_value]);
                                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
                                            if (errno != EAGAIN) {
                                                value_return = err_write();
                                            } else {
                                                send_r = FALSE;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else {                                               //resend
                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
                            if (errno != EAGAIN) value_return = err_write();
                        } else
                            send_r = TRUE;
                    }
                    if ((count == m) && send_r && (!strncmp(resp, "///", 3))) _read = TRUE;
                }
            }
            close_pipes(fd, size_pipe);
            free(fd);
            freePathList(sum);
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            //Creates char args
            strcpy(array[0], "./Q");
            sprintf(array[1], "%d", id);
            sprintf(array[2], "%d", m);
            args[0] = array[0];
            args[1] = array[1];
            args[2] = array[2];
            args[3] = NULL;

            dup2(fd[id * 4 + 2], STDIN_FILENO);
            dup2(fd[id * 4 + 1], STDOUT_FILENO);
            close_pipes(fd, size_pipe);
            free(fd);

            if (execvp(args[0], args) == -1) {
                value_return = err_exec(errno);
            }
        }
    }

    /*for (i = 0; i < m; i++) //deallocate the m-proc[]->is_open
    {
        free(proc[i].is_open);
    }
    
    free(proc); //deallocate proc*/

    return value_return;
}

/*for(i = 0; i < m; i++) {
    if(read(fd[i*4 + 0], resp, DIM_RESP) > 0) {
        //fprintf(stderr,"P read: %s\n",resp);
        if(!strcmp(resp, "///")) { //Controlla se e` la fine del messaggio
            count++; //Conta quanti terminatori sono arrivati
            if(count == m) { //Quando tutti i figli hanno terminato
                fprintf(stderr,"P: Chiudo %s\n",resp);
                _read = TRUE;                            
            }
        }
    }
}*/