#include "lib/lib.h"
int value_return = 0;  //Valore di ritorno, globale per "send_to_R"
process *p;            //Dichiaro p, viene dichiarato globalmente perché handle_sigint non può ricevere parametri al di fuori di sig
int fd1_fifo;          //A writes in R
int fd2_fifo;          //R writes in A

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

    close(fd1_fifo);
    close(fd2_fifo);

    freeList(p);         //Libero la lista di processi che ho salvato
    exit(value_return);  //Eseguo exit con codice di ritorno -1
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {  //Handler per ricezione di SIGTERM
    value_return = err_kill_process_R();                         //Ritorna il valore di errore kill processo R
}

void catch_sigterm() {  //handler per catturare il kill al di fuori del programma
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main() {                            //struttura main
    catch_sigterm();                    //avvio il ciclo di cattura del segnale di SIGTERM
    signal(SIGINT, handle_sigint);      //Handler per SIGINT (Ctrl-C)
    p = create_process(1);              //Alloca dinamicamente p con dimensione 1
    insertProcess(p, getpid());         //Inserisce il pid di R nella prima posizione della lista p
                                        //
    const char *fifo1 = "/tmp/A_to_R";  //
    const char *fifo2 = "/tmp/R_to_A";  //
    int _close = FALSE;                 //
    char cmd[DIM_CMD];                  //
    int retrieve = FALSE;               //
    int _r_write = TRUE;                //
    char resp[DIM_RESP];                //
    char path[DIM_PATH];                //
    int arr = FALSE;                    //
    int spaces;                         //
    char *dupl = NULL;                  //
    char *flag;                         //
    int p_create = FALSE;               //
    int enoent = FALSE;                 //
                                        //
    printf("Start R\n");                //

    //IPC--------------------------------------------------------------------------------------------

    if (value_return == 0) {                                           //
        printf("Waiting for A...\n");                                  //
        printf("Use " BOLDYELLOW "[CTRL+C]" RESET " to interrupt\n");  //

        if (mkfifo(fifo1, 0666) == -1) {    //Prova a creare la pipe
            if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
            }                               //
        }                                   //

        fd1_fifo = open(fifo1, O_RDONLY);                   //
        if (fd1_fifo == -1) {                               //
            value_return = err_fifo();                      //
        }                                                   //
        do {                                                //
            if (!enoent) {                                  //
                if (mkfifo(fifo2, 0666) == -1) {            //Prova a creare la pipe
                    if (errno != EEXIST) {                  //In caso di errore controlla che la pipe non fosse gia` presente
                        value_return = err_fifo();          //Ritorna errore se l'operazione non va a buon fine
                    }                                       //
                }                                           //
            }                                               //
            fd2_fifo = open(fifo2, O_WRONLY | O_NONBLOCK);  //
            if (fd2_fifo != -1) {                           //
                p_create = TRUE;                            //
            } else if (errno == ENOENT) {                   //
                enoent = FALSE;                             //
            } else if (errno != ENXIO) {                    //
                value_return = err_fifo();                  //
            }                                               //
        } while (value_return == 0 && !p_create);           //
    }                                                       //

    if (value_return == 0) {                             //
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {  //
            value_return = err_fcntl();                  //
        }                                                //
    }                                                    //

    while (value_return == 0 && !_close) {
        if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {
            if (!strncmp(cmd, "close", 5)) {
                _close = TRUE;
                printf(BOLDWHITE "R" RESET ": Closing...\n");
            } else if (!retrieve && (!strncmp(cmd, "print", 5) || !strncmp(cmd, "report", 6))) {
                retrieve = TRUE;
                _r_write = TRUE;
                checkArg(cmd, &spaces);
                if (strstr(cmd, "report") != NULL) {
                    if (spaces != 2) {
                        _r_write = FALSE;
                        retrieve = FALSE;
                        printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                        fflush(stdout);
                    } else {
                        dupl = strdup(cmd);
                        flag = strtok(dupl, " ");
                        flag = strtok(NULL, " ");
                        if (strncmp(flag, "-c", 2) && strncmp(flag, "-a", 2)) {
                            _r_write = FALSE;
                            retrieve = FALSE;
                            printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                            fflush(stdout);
                        } else {
                            strcpy(cmd, flag);
                        }
                        free(dupl);
                    }
                }
                if (strstr(cmd, "print") != NULL) {
                    if (spaces > 2) {
                        _r_write = FALSE;
                        retrieve = FALSE;
                        printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                        fflush(stdout);
                    } else if (spaces == 2) {
                        dupl = strdup(cmd);
                        flag = strtok(dupl, " ");
                        flag = strtok(NULL, " ");
                        if (strncmp(flag, "-d", 2)) {
                            _r_write = FALSE;
                            retrieve = FALSE;
                            printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n> ");
                            fflush(stdout);
                        } else {
                            strcpy(cmd, flag);
                        }
                        free(dupl);
                    }
                }
                while (value_return == 0 && _r_write) {
                    if (write(fd2_fifo, cmd, DIM_CMD) == -1) {
                        if (errno != EAGAIN) {
                            value_return = err_write();
                        }
                    } else {
                        _r_write = FALSE;
                    }
                }
            }
        }
        if (retrieve) {
            printf("\n");
            arr = FALSE;
        }
        while (retrieve) {
            if (!strncmp(cmd, "print", 5)) {
                if (read(fd1_fifo, path, DIM_PATH + 2) > 0) {
                    if (strstr(path, "#") != NULL) {
                        if (strncmp(path, "#ANALYZING", 10)) {
                            printf("%s\n", path);
                            usleep(10000);
                            arr = TRUE;
                        } else {
                            retrieve = FALSE;
                        }
                    } else if (!strncmp(path, "///", 3)) {
                        retrieve = FALSE;
                        if (!arr) printf("Lista vuota\n");
                        printf("\n> ");
                        fflush(stdout);
                    }
                }
            }
            if (!strncmp(cmd, "-c", 2)) {
                if (read(fd1_fifo, resp, DIM_RESP) > 0) {
                    if (!strncmp(resp, "#ANALYZING", 10)) {
                        retrieve = FALSE;
                    } else if (strstr(resp, ",") != NULL) {
                        printCluster(resp);
                        printf("\n> ");
                        fflush(stdout);
                        retrieve = FALSE;
                    }
                }
            }
            if (!strncmp(cmd, "-a", 2)) {
                if (read(fd1_fifo, resp, DIM_RESP) > 0) {
                    if (!strncmp(resp, "#ANALYZING", 10)) {
                        retrieve = FALSE;
                    } else if (strstr(resp, ",") != NULL) {
                        printStat(resp);
                        printf("\n> ");
                        fflush(stdout);
                        retrieve = FALSE;
                    }
                }
            }
        }
    }

    if (value_return == 0) {
        if (close(fd1_fifo) == -1) {
            value_return = err_close();
        }
        if (close(fd2_fifo) == -1) {
            value_return = err_close();
        }
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