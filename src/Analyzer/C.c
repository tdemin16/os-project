#include "../lib/lib.h"

int main(int argc, char const* argv[]) {
    int value_return = 0;
    int nfiles = 0;  //number of files to retreive from pipe
    int n = 3;
    int m = 4;
    int i;
    int j;
    int k;
    char path[PATH_MAX];
    char failedPath[PATH_MAX];

    char resp[DIM_RESP];
    char sum[DIM_RESP];
    int v[DIM_V];
    int part_received = 0;
    int count = 0;  //Maintain the current amount of files sended

    //IPC Variables
    int* fd;
    pid_t f = getpid();
    int id;         //Indica il numero del figlio (necessario per calcolare quale pipe utilizzare)
    int size_pipe;  //Size of pipes

    char arrayArgomenti[2][4];
    char* args[3];
    int _read = FALSE;   //Indica se ha finito di leggere dai figli
    int _write = FALSE;  //Indica se ha finito di scrivere

    int test = 0;
    failedPath[0] = '\0';
    char stop = FALSE;
    char end = FALSE;
    char send_r = TRUE;
    int terminated[n];
    int id_r;
    for (i = 0; i < n; i++) terminated[i] = FALSE;

    //Parsing arguments------------------------------------------------------------------------------------------
    if (argc % 2 == 0 || argc < 2) {  //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_C();
    } else {
        for (i = 1; i < argc && value_return == 0; i += 2) {
            if (!strcmp(argv[i], "-nfiles")) {  //Check if the argument is equal to -nfiles
                nfiles = atoi(argv[i + 1]);
                if (nfiles == 0) value_return = err_args_C();
            } else if (!strcmp(argv[i], "-setn")) {  //Check if argument is equal to -setn
                n = atoi(argv[i + 1]);
                if (n == 0) value_return = err_args_C();
            } else if (!strcmp(argv[i], "-setm")) {  //Check if argument is equal to -setm
                m = atoi(argv[i + 1]);
                if (m == 0) value_return = err_args_C();
            } else {
                value_return = err_args_C();
            }
        }
        if (nfiles == 0 && value_return == 0) value_return = err_args_C();  //Check if nfiles is setted, if not gives an error (value_return used to avoid double messages)
    }

    array* retrive = createPathList(nfiles);

    initialize_vector(v);

    //Generating pipes-------------------------------------------------------
    if (value_return == 0) {
        //Crea n*4 pipes (4 per coppia padre figlio, 2 in lettura e 2 in scrittura)
        size_pipe = n * 4;
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
        if (unlock_pipes(fd, size_pipe) == -1) {  //Set nonblocking pipes
            value_return = err_fcntl();
        }
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }

    //Forking----------------------------------------------------------------
    if (value_return == 0) {
        //Ciclo n volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi n processi
        for (i = 0; i < n && f > 0 && value_return == 0; i++) {
            f = fork();
            if (f == 0) {  //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
                id = i;
            } else if (f == -1) {           //Controllo che non ci siano stati errori durante il fork
                value_return = err_fork();  //In caso di errore setta il valore di ritorno a ERR_FORK
            }                               /*else{
                //insert_process(f);
            }*/
        }
    }

    //----------------------------------------------------------------------------------------
    if (value_return == 0) {
        i = 0;
        k = 0;
        if (f > 0) {  //PARENT SIDE
            while (value_return == 0 && (!_read || !_write)) {
                if (!_write) {                                             //CICLO DI SCRITTURA
                    if (count != nfiles) {                                 //Se non sono ancora tutti arraivati
                        if (stop == FALSE) {                               //E non ci troviamo in uno stato di stop per rinvio dati
                            if (read(STDIN_FILENO, path, PATH_MAX) > 0) {  //provo a leggere
                                insertPathList(retrive, path, 0);
                                //fprintf(stderr,"Aggiunto %s\n",path);
                                if (write(fd[i * 4 + 3], path, PATH_MAX) == -1) {  //Provo a scrivere
                                    if (errno != EAGAIN) {
                                        value_return = err_write();
                                        //fprintf(stderr,"errore\n");
                                    } else {  //Se da errore in scrittura copio il path in failedPath e setto lo stato di stop (Retransmit)
                                        stop = TRUE;
                                        strcpy(failedPath, path);
                                        //fprintf(stderr,"C->P: Pipe per %d piena, %s aspetta\n",i,path);
                                    }
                                } else {  //scritto con successo
                                    //fprintf(stderr,"C->P: assegno a %d %s\n",i,path);
                                    count++;          //Tengo conto della scrittura
                                    i = (i + 1) % n;  //Usato per ciclare su tutte le pipe in scrittura
                                }
                            }
                        } else {
                            //fprintf(stderr,"STOP TRUE\n");
                            if (write(fd[i * 4 + 3], failedPath, PATH_MAX) == -1) {  //Test write
                                if (errno != EAGAIN) {
                                    value_return = err_write();
                                } else {
                                    //fprintf(stderr,"C->P: Pipe per %d ancora piena, %s aspetta\n",i,failedPath);
                                }
                                //ADD SIGNAL HANDLING
                            } else {
                                stop = FALSE;  //Se la scrittura va a buon fine esco dallo stato di stop
                                count++;       //Tengo conto dell'invio
                                i = (i + 1) % n;
                            }
                        }
                    } else {  //Se tutti i file sono stati ricevuti allora devo inviare una stringa di terminazione: ///
                        strcpy(path, "///");
                        end = TRUE;                //Setto end = true, se non ci sono problemi rimarrà true
                        for (j = 0; j < n; j++) {  //Manda a tutti i processi P la fine della scrittura

                            if (!terminated[j]) {  //Se non è ancora stato mandato a quel processo
                                if (write(fd[j * 4 + 3], path, PATH_MAX) == -1) {
                                    if (errno != EAGAIN) {
                                        value_return = err_write();
                                    } else {
                                        terminated[j] = FALSE;  //Se non riesce a mandare
                                        end = FALSE;
                                    }
                                } else {
                                    //fprintf(stderr,"C->P: Invio /// a %d\n",j);
                                    terminated[j] = TRUE;  //Se riesce a mandare
                                }
                            }
                            //for (i= 0; i<n; i++){
                            //fprintf(stderr,"%d ",terminated[i]);
                            //}
                            //fprintf(stderr,"\n");
                        }
                        //end = TRUE;
                        if (end == TRUE) {
                            _write = TRUE;  //Finito di scrivere
                        }
                    }
                }

                //Read
                if (!_read) {
                    if (send_r) {
                        if (read(fd[k * 4 + 0], resp, DIM_RESP) > 0) {
                            if (!strncmp(resp, "///", 3)) {  //Lascia questo blocco
                                part_received++;
                                if (part_received == n) {
                                    _read = TRUE;
                                }
                            } else {
                                if (strstr(resp, "#") != NULL) {
                                    id_r = atoi(strtok(strdup(resp), "#"));
                                    retrive->analyzed[id_r] = 1;
                                    if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {
                                        if (errno != EAGAIN) {
                                            value_return = err_write();
                                        } else {
                                            send_r = FALSE;
                                        }
                                    }
                                    
                                }
                            }
                        }
                    } else {
                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {
                            if (errno != EAGAIN) value_return = err_write();
                        } else
                            send_r = TRUE;
                    }
                    k = (k + 1) % n;
                }
            }
            close_pipes(fd, size_pipe);
            free(fd);
            //printPathList(retrive);
            freePathList(retrive);
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            //Creates char args
            strcpy(arrayArgomenti[0], "./P");
            sprintf(arrayArgomenti[1], "%d", m);
            args[0] = arrayArgomenti[0];
            args[1] = arrayArgomenti[1];
            args[2] = NULL;

            dup2(fd[id * 4 + 2], STDIN_FILENO);
            dup2(fd[id * 4 + 1], STDOUT_FILENO);
            close_pipes(fd, size_pipe);
            free(fd);

            if (execvp(args[0], args) == -1) {   //Test exec
                value_return = err_exec(errno);  //Set value return
            }
        }
    }

    /*for (i = 0; i < n; i++) //deallocate the n-proc[]->is_open
    {
        free(proc[i].is_open);
    }

    free(proc); //deallocate proc
    */
    return value_return;
}

//NON NECESSARIO, MANTENUTO PER SICUREZZA---------------------------------------------------
/*if(value_return == 0) {
    if(f > 0) { //PARENT SIDE
        fileLeft = nfiles;
        processLeft = n;
        tmpFiles = 0;

        while (processLeft >0){ //Ciclo fino a che non ho assegnato i file a ogni processo
            printf("Process %d: %d files\n",n-processLeft+1,(int)((float)fileLeft/(float)processLeft));

            tmpFiles = (int)((float)fileLeft/(float)processLeft); //Divido i file rimanenti per i processi rimanenti e tengo il numero troncato
            for (i = 0; i<tmpFiles; i++){ //Leggo il numero di file assegnato al processo
                if(read(STDIN_FILENO, path, PATH_MAX) == 0) {

                }
            }

            fileLeft-=tmpFiles;
            processLeft-=1;
        }
    }
}*/