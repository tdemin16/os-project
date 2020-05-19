#include "../lib/lib.h"

process *p;  //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)

void handle_sigint(int sig) {
    printf("\n[!] Ricevuta terminazione, inizio terminazione processi ... \n");
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

    p = create_process(1);  //Allocate dynamically p with dimension 1

    int value_return = 0;  //Valore di ritorno, globale per "send_to_R"

    //COMMUNICATION WITH R
    const char *fifo = "/tmp/A_R_Comm";  //Nome fifo con R
    int fd_fifo;                         //pipe fifo con R

    //COMMUNICATION WITH M
    char cmd[DIM_CMD];  //Comando rivevuto da M
    int _close = FALSE;

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;

    //Utility
    int i;            //Variabile usata per ciclare gli argomenti (argv[i])
    int count = 0;    //numero di file univoci da analizzare
    int perc = 0;     //Ricevimento parziale file
    int oldperc = 0;  //Parziale precedente
    char *tmp = NULL;
    char *tmpResp = NULL;
    char *tmpPercorso = NULL;

    array *lista = createPathList(10);  //Nuova lista dei path

    //Variables for IPC
    int fd_1[2];  //Pipes
    int fd_2[2];
    pid_t f;              //fork return value
    char array[7][20];    //Matrice di appoggio
    char *args[8];        //String og arguments to pass to child
    int _write = FALSE;   //true when finish writing the pipe
    int _read = FALSE;    //true when fisnish reading from pipe
    char resp[DIM_RESP];  //Stringa in cui salvare i messaggi ottenuti dal figlio
    int id_r;             //Id file ricevuto
    char *resp_val;       //Messaggio senza Id
    char *file;           //Messaggio senza Id e identificatori (#)
    int firstVal = 0;     //Controllo sulla validita' di un messaggio
    char sum[DIM_RESP];
    int v[DIM_V];          //Array con valori totali
    int notAnalyzed = 0;   //Flag indicante se e` avvenuta o meno la lettura della pipe
    initialize_vector(v);  //Inizializzazione vettore dei valori totali

    value_return = parser(argc, argv, lista, &count, &n, &m);  //Controlla i parametri passati ad A

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
    if (value_return == 0) {
        if (mkfifo(fifo, 0666) == -1) {     //Prova a creare la pipe
            if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
            }
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
        fd_fifo = open(fifo, O_RDONLY | O_NONBLOCK);  //Prova ad aprire la pipe in scrittura
        if (fd_fifo == -1) {                          //Error handling
            value_return = err_file_open();           //Errore nell'apertura del file
        }
    }

    if (value_return == 0) {
        f = fork();                     //Fork dei processi
        if (f == -1) {                  //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork();  //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    //------------------------------------------------------------------------------

    if (value_return == 0) {
        if (f > 0) {              //PARENT SIDE
            insertProcess(p, f);  //Insert child process in list p
            i = 0;
            while (value_return == 0 && (!_read || !_write || !_close)) {  //cicla finche` non ha finito di leggere e scrivere o avviene un errore

                //M Work in progress
                _close = TRUE;
                if (!_close) {
                    if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {
                        printf("%s\n", cmd);
                    }
                }

                //Write
                if (!_write) {                                                     //Esegue il blocco finche` non ha finito di scrivere
                    if (write(fd_1[WRITE], lista->pathList[i], PATH_MAX) == -1) {  //Prova a scrivere sulla pipe
                        if (errno != EAGAIN) {                                     //Se avviene un errore e non e` causato dalla dimensione della pipe
                            value_return = err_write();                            //Ritorna l'errore sulla scrittura
                        }
                    } else {
                        i++;                //passa al prossimo elemento della lista
                        if (i == count) {   //Qunado ha finito di inviare
                            _write = TRUE;  //Setta il flag a true
                        }
                    }
                }

                //Read
                if (!_read) {                                    //Esegue il blocco fiche` non c'e` piu` nulla nella pipe
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
                            if ((int)((float)perc * 10 / (float)count) > oldperc) {
                                oldperc = (int)((float)perc * 10 / (float)count);
                                system("clear");
                                percAvanzamento(perc, count);
                            }

                            if (perc == count) {
                                _read = TRUE;
                                system("clear");
                                printf("Numero file analizzati: %d\nProcessi:%d\nSezioni:%d\n\n", count, n, m);
                                arrayToCsv(v, sum);
                                printStat_Cluster(sum);
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
                if (close(fd_fifo) == -1) {
                    value_return = err_close();
                }
            }

            //Elimina la fifo
            if (value_return == 0) {
                if (unlink(fifo) == -1) {
                    value_return = err_unlink();
                }
            }
            freePathList(lista);
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE

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
        }
    }

    freeList(p);

    return value_return;
}