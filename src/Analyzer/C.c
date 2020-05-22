#include "../lib/lib.h"

int value_return = 0;
void sig_term_handler(int signum, siginfo_t* info, void* ptr) {
    value_return = err_kill_process_C();
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
    int nfiles = 0;  //number of files to retreive from pipe
    int n = 3;
    int m = 4;
    int i;
    int k;

    char path[PATH_MAX];        //Paths da mandare ai figli
    char failedPath[PATH_MAX];  //Percorsi non inviati a causa della pipe piena
    char resp[DIM_RESP];    //Stringa con i valori ricevuta dai figli
    int count = 0;          //Maintain the current amount of files sended

    //IPC Variables
    int* fd;             //pipes
    pid_t f = getpid();  //pid (utilizzato per generare i figli)
    int id;              //Indica il numero del figlio (necessario per calcolare quale pipe utilizzare)
    int size_pipe;       //Numero pipe * 4 (2 READ 2 WRITE)

    int _read = FALSE;          //Indica se ha finito di leggere dai figli
    int _write = FALSE;         //Indica se ha finito di scrivere
    int _close = FALSE;
    failedPath[0] = '\0';  //Inizializza la stringa failed path
    char stop = FALSE;     //Bloccano la ricezione di nuovi dati dai figli
    char send_r = TRUE;    //Controlla la dimensione della pipe del padre
    int oldfl;             //usato per togliere la O_NONBLOCK dai flag
    int pendingPath = 0;
    

    //Parsing arguments------------------------------------------------------------------------------------------
    if (argc % 2 == 0 || argc < 2) {  //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_C();  //Ritorna errore sugli argomenti
    } else {
        for (i = 1; i < argc && value_return == 0; i += 2) {   //Loop through argv
            if (!strcmp(argv[i], "-nfiles")) {                 //Check if the argument is equal to -nfiles
                nfiles = atoi(argv[i + 1]);                    //Converte la stringa in intero
                if (nfiles == 0) value_return = err_args_C();  //Ritorna errore sugli argomenti
            } else if (!strcmp(argv[i], "-setn")) {            //Check if argument is equal to -setn
                n = atoi(argv[i + 1]);                         //Converte la stringa in intero
                if (n == 0) value_return = err_args_C();       //Ritorna errore sugli argomenti
            } else if (!strcmp(argv[i], "-setm")) {            //Check if argument is equal to -setm
                m = atoi(argv[i + 1]);                         //Conversione
                if (m == 0) value_return = err_args_C();       //Errore
            } else {                                           //Se compare qualcos'altro e` sicuramente sbagliato
                value_return = err_args_C();                   //Errore
            }
        }
        if (nfiles == 0 && value_return == 0) value_return = err_args_C();  //Check if nfiles is setted, if not gives an error (value_return used to avoid double messages)
    }

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
            char str[15];
            sprintf(str, "%d.txt", getpid());
            FILE* debug = fopen(str, "a");
            fprintf(debug, "AVVIATO C\n");
            fclose(debug);
            while (value_return == 0 && (!_close)) {  //Cicla finche` non ha finito di leggere o scrivere o va in errore
                //write
                if (!_write) {                                         //CICLO DI SCRITTURA
                    if (stop == FALSE) {                               //E non ci troviamo in uno stato di stop per rinvio dati
                        if (read(STDIN_FILENO, path, PATH_MAX) > 0) {  //provo a leggere
                            pendingPath++;
                            if (pendingPath == 1) {
                                if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
                                    value_return = err_fcntl();
                                }
                            }
                            debug = fopen(str, "a");
                            fprintf(debug, "C: LEGGO %s\n", path);
                            fclose(debug);
                            if (!strncmp(path, "#", 1)) {
                                nClearAndClose(fd, n);              //Svuota le pipe in discesa e manda #CLOSE
                                if (!strncmp(path, "#CLOSE", 6)) {  //Se leggo una stringa di terminazione
                                    _close = TRUE;                  //Setto end a true
                                } else if (!strncmp(path, "#SET", 4)) {
                                    parseOnFly(path, &n, &m);  //Estrae n e m dalla stringa #SET#N#M#
                                    size_pipe = n * 4;
                                    fd = (int*)realloc(fd, size_pipe * sizeof(int));
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
                                    forkC(&n, &f, &id, &value_return);
                                    if (f == 0) execC(&m, &f, &id, fd, &value_return, &size_pipe);
                                }
                                pendingPath--;
                            } else {
                                if (write(fd[i * 4 + 3], path, PATH_MAX) == -1) {  //Provo a scrivere
                                    if (errno != EAGAIN) {                         //Controlla che non sia una errore di pipe piena
                                        value_return = err_write();                //Setta il valore di ritorno
                                    } else {                                       //Se da errore in scrittura copio il path in failedPath e setto lo stato di stop (Retransmit)
                                        stop = TRUE;
                                        strcpy(failedPath, path);
                                    }
                                } else {  //scritto con successo
                                    debug = fopen(str, "a");
                                    fprintf(debug, "C: Inviato a %d: %s\n", i, path);
                                    fclose(debug);
                                    count++;          //Tengo conto della scrittura
                                    i = (i + 1) % n;  //Usato per ciclare su tutte le pipe in scrittura
                                }
                            }
                        }
                    } else {                                                     //Se c'e` uno stop sull'invio dei dati
                        if (write(fd[i * 4 + 3], failedPath, PATH_MAX) == -1) {  //Test write
                            if (errno != EAGAIN) {                               //Controlla che non sia una errore di pipe piena
                                value_return = err_write();                      //Setta il valore di ritorno
                            }
                        } else {
                            debug = fopen(str, "a");
                            fprintf(debug, "C: Inviato a %d: %s\n", i, failedPath);
                            fclose(debug);
                            stop = FALSE;     //Se la scrittura va a buon fine esco dallo stato di stop
                            count++;          //Tengo conto dell'invio
                            i = (i + 1) % n;  //Incremento i in maniera ciclica
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
                        /* handle error */
                    }
                    fcntl(STDIN_FILENO, F_SETFL, oldfl & ~O_NONBLOCK);
                }
            }
            if (f > 0) {                     //Se e' un processo figlio,non deve liberare le pipe
                close_pipes(fd, size_pipe);  //Chiude tutte le pipes
                free(fd);                    //Libera la memoria delle pipes
            }
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            execC(&m, &f, &id, fd, &value_return, &size_pipe);
        }
    }

    return value_return;
}