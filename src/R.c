#include "lib/lib.h"
int value_return = 0;  //Valore di ritorno, globale per "send_to_R"
process *p;            //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)
int fd_fifo;           //pipe fifo con R

void handle_sigint(int sig) {
    printf("\n[!] Ricevuta terminazione per R, inizio terminazione processo ... \n");
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
    if (!close(fd_fifo))  //close fifo
    {
        printf("[!] Chiusura fifo completata\n");
    }
    freeList(p);  //free memory allocated for p
    printf("[!] ... Chiusura processo terminata\n");
    exit(-1);  //return exit with error -1
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    value_return = err_kill_process_R();
}

void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main() {
    catch_sigterm();
    signal(SIGINT, handle_sigint);  //Handler for SIGINT (Ctrl-C)
    p = create_process(1);  //Allocate dynamically p with dimension 1
    insertProcess(p,getpid());

    const char* fifo = "/tmp/A_R_Comm";
    int _close = FALSE;
    char cmd[DIM_CMD];
    int retrieve = FALSE;
    int _write_val = -1;
    char resp[DIM_RESP];

    printf("Start R\n");

    //IPC--------------------------------------------------------------------------------------------
    if (value_return == 0) {
        if (mkfifo(fifo, 0666) == -1) {     //Prova a creare la pipe
            if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
            }
        }
    }

    if (value_return == 0) {
        do {
            fd_fifo = open(fifo, O_WRONLY | O_NONBLOCK);  //Prova ad aprire la pipe in scrittura
            if (fd_fifo == -1) {                          //Error handling
                if (errno != ENXIO) {                     //Se errno == 6, il file A non e' stato ancora aperto
                    value_return = err_file_open();       //Errore nell'apertura del file
                }
            }
        } while (value_return == 0 && fd_fifo == -1);
    }

    if (value_return == 0) {
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }

    while (value_return == 0 && !_close) {
        if (!retrieve) {
            if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {
                if (!strncmp(cmd, "close", 5)) {
                    _close = TRUE;
                    printf("R: Closing...\n");
                } else {
                    if (!strncmp(cmd, "-c", 2) /*&& altri flags*/) {
                        do {
                            _write_val = write(fd_fifo, cmd, DIM_CMD);
                            if (_write_val == -1) {
                                if (errno == EAGAIN) {
                                    _write_val = EAGAIN;
                                } else {
                                    value_return = err_write();
                                }
                            }
                        } while (value_return == 0 && errno == EAGAIN && _write_val == -1);

                        if (value_return == 0) {
                            retrieve = TRUE;
                            if (close(fd_fifo) == -1) {
                                value_return = err_close();
                            }
                            //Open fifo in nonblocking read mode
                            if (value_return == 0) {
                                fd_fifo = open(fifo, O_RDONLY | O_NONBLOCK);  //Prova ad aprire la pipe in scrittura
                                if (fd_fifo == -1) {                          //Error handling
                                    value_return = err_file_open();           //Errore nell'apertura del file
                                }
                            }
                        }
                    }
                }
            }
        } else {
            //Ciclare sulla read
            //Scrivere in output il risultato
            if (read(fd_fifo, resp, DIM_RESP) > 0) {
                retrieve = FALSE;
                printf("%s\n", resp);
                if (close(fd_fifo) == -1) {
                    value_return = err_close();
                }
                //Open fifo in nonblocking write mode
                if (value_return == 0) {
                    do {
                        fd_fifo = open(fifo, O_WRONLY | O_NONBLOCK);  //Prova ad aprire la pipe in scrittura
                        if (fd_fifo == -1) {                          //Error handling
                            if (errno != ENXIO) {                     //Se errno == 6, il file A non e' stato ancora aperto
                                value_return = err_file_open();       //Errore nell'apertura del file
                            } else {
                                //fprintf(stderr, "ENXIO");
                            }
                        }
                    } while (value_return == 0 && fd_fifo == -1);
                }
            }
        }
    }

    if (value_return == 0) {
        if (close(fd_fifo) == -1) {
            value_return = err_close();
        }
    }

    if (value_return == 0) {
        //if (unlink(fifo) == -1) {
        //    value_return = err_unlink();
        //}
    }
    freeList(p);

    return value_return;
}

//int v[DIM_V] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94};
//char * test = "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94";
//char * test2 = "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94";
//int max_v[DIM_V] = {2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647};
//int * max = "1044,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647";
//printStat_Cluster(strdup(test));
//printStat(strdup(test));
//printInfoCluster();
//char tmp[DIM_RESP];
//char tmp[DIM_RESP];
//char * prova = "0#/home/luigi/Scrivania/LABSO/os-project/src/Analyzer/P.c";
//char * id = strtok(strdup(prova),"#");
//char * analyze = strtok(NULL,"#");
//printf("%s\n",id);
//printf("%s\n",analyze);
// ottengo v
//createCsv(v,tmp,id);
//printf("%s\n",tmp);