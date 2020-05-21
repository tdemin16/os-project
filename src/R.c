#include "lib/lib.h"

int main() {
    int value_return = 0;
    const char* fifo = "/tmp/A_R_Comm";
    int fd_fifo;
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