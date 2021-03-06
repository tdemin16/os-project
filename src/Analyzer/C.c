#include "../lib/lib.h"

int value_return = 0;
void sig_term_handler(int signum, siginfo_t* info, void* ptr) {
    value_return = err_kill_process();
    close_all_process();
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
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int i;
    int j;
    int k;

    char path[DIM_PATH];  //Paths da mandare ai figli
    memset(path, '\0', sizeof(char) * DIM_PATH);
    char failedPath[DIM_PATH];  //Percorsi non inviati a causa della pipe piena
    memset(failedPath, '\0', sizeof(char) * DIM_PATH);
    char resp[DIM_RESP];  //Stringa con i valori ricevuta dai figli
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    int count = 0;  //Maintain the current amount of files sended

    //IPC Variables
    int* fd;             //pipes
    pid_t f = getpid();  //pid (utilizzato per generare i figli)
    int id;              //Indica il numero del figlio (necessario per calcolare quale pipe utilizzare)
    int size_pipe;       //Numero pipe * 4 (2 READ 2 WRITE)

    int _read = FALSE;   //Indica se ha finito di leggere dai figli
    int _write = FALSE;  //Indica se ha finito di scrivere
    int _close = FALSE;
    failedPath[0] = '\0';  //Inizializza la stringa failed path
    char stop = FALSE;     //Bloccano la ricezione di nuovi dati dai figli
    char send_r = TRUE;    //Controlla la dimensione della pipe del padre
    int oldfl;             //usato per togliere la O_NONBLOCK dai flag
    int pendingPath = 0;
    
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
        -----------------------------------------------------------
    */

    if (value_return == 0) {
        if (unlock_pipes(fd, size_pipe) == -1) {  //Set nonblocking pipes
            value_return = err_fcntl();           //Gestione errore sullo sblocco pipe
        }
    }

    //Forking----------------------------------------------------------------
    if (value_return == 0) {
        //Ciclo n volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi n processi
        for (i = 0; i < n && f > 0 && value_return == 0; i++) {
            f = fork();
            if (f == 0) {
                id = i;                     //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
            } else if (f == -1)             //Controllo che non ci siano stati errori durante il fork
                value_return = err_fork();  //In caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    //----------------------------------------------------------------------------------------
    if (value_return == 0) {
        i = 0;
        k = 0;
        j = 0;
        if (f > 0) {                                                   //PARENT SIDE
            while (value_return == 0 && (!_close)) {                   //Cicla finche` non ha finito di leggere o scrivere o va in errore
                if (!_write) {                                         //CICLO DI SCRITTURA
                    if (stop == FALSE) {                               //E non ci troviamo in uno stato di stop per rinvio dati
                        if (read(STDIN_FILENO, path, DIM_PATH) > 0) {  //provo a leggere
                            
                            if (!strncmp(path, "#", 1)) {
                                if (!strncmp(path, "#CLOSE", 6)) {
                                    _read = TRUE;
                                    _close = TRUE;
                                    nClearAndClose(fd, n);
                                } else if (!strncmp(path, "#SET#", 5) || !strncmp(path, "#SETN", 5)) {
                                    j = 0;
                                    k = 0;
                                    nClearAndClose(fd, n);  //mando #CLOSE alle n pipe
                                    while (wait(NULL) > 0)  //Aspetto che vengano chiusi
                                        ;
                                    while (read(STDOUT_FILENO, resp, DIM_RESP) > 0)
                                        ;
                                    //nCleanSon(fd, n);
                                    close_pipes(fd, size_pipe);
                                    parseOnFly(path, &n, &m);  //Estrae n e m dalla stringa #SET#N#M#
                                    size_pipe = n * 4;
                                    free(fd);
                                    fd = (int*)malloc(size_pipe * sizeof(int));
                                    if (createPipe(fd, size_pipe) != 0) {
                                        value_return = err_pipe();
                                    }
                                    if (unlock_pipes(fd, size_pipe) == -1) {  //Set nonblocking pipes
                                        value_return = err_fcntl();           //Gestione errore sullo sblocco pipe
                                    }
                                    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {  //Sblocca lo stdin (teoricamente non necessario)
                                        value_return = err_fcntl();                  //Gestione errore sullo sblocco pipe
                                    }
                                    forkC(&n, &f, &id, &value_return);
                                    if (f == 0) {
                                        execC(&m, &f, &id, fd, &value_return, &size_pipe);
                                    }
                                    if (!sendCheck()) {
                                        value_return = err_write();
                                    }

                                } else if (!strncmp(path, "#SETM#", 6)) {
                                    j = 0;
                                    k = 0;
                                    
                                    mParseOnFly(path, &m);
                                    mSendOnFly(fd, n, m);
                                    while (read(STDOUT_FILENO, resp, DIM_RESP) > 0)
                                        ;
                                    send_r = TRUE;
                                    readCheck(fd, n);    //Aspetta qui finchè non legge check da tutti i figli
                                    if (!sendCheck()) {  //Invia il check al padre
                                        value_return = err_write();
                                    }
                                }
                                pendingPath = 0;
                            } else {  //Se si tratta di un percorso
                                pendingPath++;
                                if (pendingPath == 1) {
                                    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
                                        value_return = err_fcntl();
                                    }
                                }
                                if (write(fd[j * 4 + 3], path, DIM_PATH) == -1) {  //Provo a scrivere
                                    if (errno != EAGAIN) {                         //Controlla che non sia una errore di pipe piena
                                        value_return = err_write();                //Setta il valore di ritorno
                                    } else {                                       //Se da errore in scrittura copio il path in failedPath e setto lo stato di stop (Retransmit)
                                        stop = TRUE;
                                        strcpy(failedPath, path);
                                    }
                                } else {              //scritto con successo
                                    count++;          //Tengo conto della scrittura
                                    j = (j + 1) % n;  //Usato per ciclare su tutte le pipe in scrittura
                                }
                            }
                        }
                    } else {                                                     //Se c'e` uno stop sull'invio dei dati
                        if (write(fd[j * 4 + 3], failedPath, DIM_PATH) == -1) {  //Test write
                            if (errno != EAGAIN) {                               //Controlla che non sia una errore di pipe piena
                                value_return = err_write();                      //Setta il valore di ritorno
                            }
                        } else {
                            stop = FALSE;     //Se la scrittura va a buon fine esco dallo stato di stop
                            count++;          //Tengo conto dell'invio
                            j = (j + 1) % n;  //Incremento i in maniera ciclica
                        }
                    }
                }

                //Read
                if (!_read) {
                    if (send_r) {                                                  //Coontrolla se non ci sonon valori non inviati
                        if (read(fd[k * 4 + 0], resp, DIM_RESP) > 0) {             //Prova a leggere dalla pipe
                            if (strstr(resp, "#") != NULL) {                       //Controlla che nella stringa sia contenuto il carattere #
                                if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Prova a scrivere sulla pipe del padre
                                    if (errno != EAGAIN) {                         //Controlla che non sia una errore di pipe piena
                                        value_return = err_write();                //Manda l'errore di write
                                    } else {                                       //Caso in cui la pipe era piena
                                        send_r = FALSE;                            //Passa al reinvio
                                    }
                                } else {
                                    pendingPath--;
                                }
                            }
                        }
                    } else {
                        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {     //Prova il reinvio dei dati al padre
                            if (errno != EAGAIN) value_return = err_write();  // Controlla che non ci sia un errore di pipe piena
                        } else {
                            send_r = TRUE;  //Se ha inviato passa al prossimo elemento
                            pendingPath--;
                        }
                    }
                    k = (k + 1) % n;  //Cicla tra le pipes
                }

                if (pendingPath == 0) {
                    oldfl = fcntl(STDIN_FILENO, F_GETFL);
                    if (oldfl == -1) {
                    }
                    fcntl(STDIN_FILENO, F_SETFL, oldfl & ~O_NONBLOCK);
                }
            }

            while (wait(NULL) > 0)
                ;
            while (read(STDOUT_FILENO, resp, DIM_RESP) > 0)  //Svuota lo stdout
                ;
            close_pipes(fd, size_pipe);  //Chiude tutte le pipes
            free(fd);                    //Libera la memoria delle pipes
        }
    }

    if (value_return != 0) {
        close_all_process();
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            execC(&m, &f, &id, fd, &value_return, &size_pipe);
        }
    }

    return value_return;
}