#include "../lib/lib.h"

int value_return = 0;  //Valore di ritorno, globale per "send_to_R"
process *p;            //Dichiaro p (è globale perché handle_sigint non può avere parametri)
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

    freeList(p);  //Libero la lista di processi che ho salvato
    printf("\n");
    exit(value_return);  //Eseguo exit con codice di ritorno -1
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {  //handler per SIGTERM
    value_return = err_kill_process_A();                         //Nel caso accada ritorna errore processo A killato al di fuori del programma
}

void catch_sigterm() {  //Funzione per consentire l'acquisizione del SIGTERM
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char *argv[]) {  //Main
    printf("Start A\n");            //Stampo l'esecuzione di A
    catch_sigterm();                //Avvio l'handler per SIGTERM
    signal(SIGINT, handle_sigint);  //Handler per SIGINT (Ctrl-C)
                                    //
    p = create_process(1);          //Allocate dynamically p with dimension 1

    //COMMUNICATION WITH R
    const char *fifo1 = "/tmp/A_to_R";  //Nome fifo A con R
    const char *fifo2 = "/tmp/R_to_A";  //Nome fifo R con A
    int retrieve = TRUE;                //Indica se deve rucperare dati da mandare ad R
    int p_create = FALSE;               //Indica se la pipe fifo e` stata creata
    char print_method[DIM_CMD];         //Comando inviato da R ad A
    char type_resp[DIM_RESP];           //Set di dati da dare ad R
    char tmp_resp[DIM_PATH];            //Fine lettura di R
    strcpy(tmp_resp, "///");

    //COMMUNICATION WITH M - STDIN
    char cmd[DIM_CMD];   //Comando rivevuto da M
    int _close = FALSE;  //Indica se A deve chiudersi o continuare l'esecuzione
    char *new_n;         //Nuovo n dopo il comando set. Serve a controllare la correttezza del valore prima di riavviare i processi
    char *new_m;         //Nuovo m dopo il comando set. Serve a controllare la correttezza del valore prima di riavviare i processi
    char *dupl = NULL;   //Stringa di appoggio per il parsing del comando set n m

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;  //Numero di processi P
    int m = 4;  //Numero di processi Q

    //Utility
    int i;
    int j;
    int count = 0;     //numero di file univoci da analizzare
    int perc = 0;      //Ricevimento parziale file
    char n_exec[12];   //Stringa di appoggio per creare gli argomenti da dare a C
    char m_exec[12];   //Stringa di appoggio per creare gli argomenti da dare a C
    struct stat attr;  //Necessaria per aggiornare last_edit di un file

    char analyzing = FALSE;    //Indica se i figli stanno eseguendo l'analisi
    int pathSent = 0;          //Indica quanti percorsi sono stati mandati
    char *tmp = NULL;          // | Stringe di appoggio
    char *tmpResp = NULL;      // | per la manipolazione
    char *tmpPercorso = NULL;  // | delle stringhe dei
    char **tempPath;           // | percorsi
    int vReturn;               //Numero di file aggiunti o rimossi da parser
    int duplicate = 0;         //Conta i duplicati non aggiunti dal parser

    array *lista = createPathList(10);  //Nuova lista dei path

    //Variables for IPC
    int fd_1[2];          //Pipes
    int fd_2[2];          //
    pid_t f;              //fork return value
    int _write = TRUE;    //true when finish writing the pipe
    int _read = TRUE;     //true when fisnish reading from pipe
    char resp[DIM_RESP];  //Stringa in cui salvare i messaggi ottenuti dal figlio
    int id_r;             //Id file ricevuto
    char *resp_val;       //Messaggio senza Id
    char *file;           //Messaggio senza Id e identificatori (#)
    int firstVal = 0;     //Controllo sulla validita' di un messaggio
    char sum[DIM_RESP];
    long v[DIM_V];        //Array con valori totali
    int notAnalyzed = 0;  //Flag indicante se e` avvenuta o meno la lettura della pipe
    int argCounter = 0;
    int totFiles = 0;
    initialize_vector(v);  //Inizializzazione vettore dei valori totali
    int val = 0;
    time_t start, end;
    double elapsed;
    if (argc > 1) {                                                              //Caso in cui ci sono degli argomenti all'avvio
        val = parser2(argc, argv, lista, &count, &n, &m, &vReturn, &duplicate);  //Parsing degli argomenti
        if (val == 0) {                                                          //Controlla i parametri passati ad A
            if (vReturn > 0) {                                                   //Vuol dire che c'è stato un errore nell'inserimento di PathList
                _write = FALSE;                                                  //Non può scrivere
                time(&start);
            }
        } else {
            system("clear");                                  //Libera il terminale
            printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n");  //Avvisa l'inizio dell'Analyzer
            if (val == 1) err_args_A();                       //Se val == 1 allora c'è errore argomenti di A non sufficienti
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

        fd1_fifo = open(fifo1, O_WRONLY);  //Prova ad aprire la pipe in scrittura
        if (fd1_fifo == -1) {              //Error handling
            value_return = err_fifo();     //value_return assume valore di errore della fifo
        }

        do {
            if (mkfifo(fifo2, 0666) == -1) {    //Prova a creare la pipe
                if (errno != EEXIST) {          //In caso di errore controlla che la pipe non fosse gia` presente
                    value_return = err_fifo();  //Ritorna errore se l'operazione non va a buon fine
                }
            }
            fd2_fifo = open(fifo2, O_RDONLY | O_NONBLOCK);  //Apre la fifo2 in modalità solo lettura
            if (fd2_fifo != -1) {                           //Se diverso da -1
                p_create = TRUE;                            //Allora p_create = true
            } else if (errno != ENOENT) {                   //Altrimenti se diverso da ENOENT
                value_return = err_fifo();                  //value_return prende errore fifo
            }
        } while (value_return == 0 && !p_create);  //Cicla fino a quando value_return == 0 e p_create è falso
    }

    system("clear");                                    //pulisce il terminale
    printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n\n");  //Avvisa l'avvio di analyzer
    fflush(stdout);                                     //libera il buffer

    if (value_return == 0 && !_close) {  //Se non ci sono stati errori e il flag close non è a true (_close serve per la chiusura dei programmi)
        f = fork();                      //Fork dei processi
        if (f == -1) {                   //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork();   //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    //------------------------------------------------------------------------------

    if (value_return == 0 && !_close) {
        if (f > 0) {              //PARENT SIDE
            insertProcess(p, f);  //Inserisce processo figlio f nella lista processi p
            i = 0;
            if (lista->count == 0) {
                printf("> ");
                fflush(stdout);
            }

            while (value_return == 0 && !_close) {  //cicla finche` non ha finito di leggere e scrivere o avviene un errore
sleep(1);
                //M - STDIN
                if (!_close) {                                   //Controlla close non sia già settato a true
                    if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {  //La read non ha errori
                        if (!strncmp(cmd, "close", 5)) {         //Verifica se cmd è close, in tal caso prosegue alla chiusura dei processi
                            closeAll(fd_1);                      //chiudi tutti i processi figli
                            while (wait(NULL) > 0)               //Attende che tutti i processi figli siano chiusi prima di terminare
                                ;
                            _close = TRUE;  //flag close settato a TRUE, obbliga il programma a terminare
                            printf(BOLDWHITE "A" RESET ": Closing...\n");
                        }

                        if (!strncmp(cmd, "add", 3)) {                                                       //Se invece il comando è "add"
                            if (!analyzing) {                                                                //Verifica che non stia già analizzando
                                if (strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL) {          //Mentra analizza controlla se l'utente cambia setn o setm
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\n");  //in tal caso verifica se sono correttamente inseriti
                                    printf("Usa help per vedere la lista di comandi utilizzabili.\n\n");     //Stampa errore se sono stati inseriti comandi errati
                                    fflush(stdout);                                                          //Libera il buffer
                                } else if (checkArg(cmd, &argCounter)) {                                     //Verifica gli argomenti inseriti a comando
                                    tempPath = malloc(argCounter * sizeof(char *));                          //Alloca memoria a tempPath
                                    for (j = 0; j < argCounter; j++) {                                       //Cicla da 0 al numero di argomenti
                                        tempPath[j] = malloc(DIM_PATH * sizeof(char));                       //Alloca a tempPath la dimensione DIM_PATH
                                    }
                                    strcpy(tempPath[0], strtok(cmd, " "));       //Copia il comando cmd in tempPath[0] (cmd è il comando senza spazi)
                                    for (j = 1; j < argCounter; j++) {           //Cicla da 0 al numero di argomenti
                                        strcpy(tempPath[j], strtok(NULL, " "));  //Stessa copia della riga superirore
                                    }
                                    if (parser2(argCounter, tempPath, lista, &count, &n, &m, &vReturn, &duplicate) == 0) {  //Se il parsing funziona (=> i file sono stati aggiunti correttamente)
                                        printf("\n" WHITE "Aggiunti" RESET " %d files\n", vReturn);                         //Stampo il corretto parsing
                                    } else {
                                        //err_args_A();
                                    }
                                    for (j = 0; j < argCounter; j++) {  //Cicla da 0 al numero di argomenti
                                        free(tempPath[j]);              //Libera la lista tempPath[j]
                                    }
                                    free(tempPath);  //Libera tempPath
                                } else {             //Altrimenti (se non ci sono setn e/o setm e non vengono inseriti comandi corretti)
                                    err_args_A();    //Ritorna errore degli argomenti di A
                                }
                            } else {  //Altrimenti l'analisi è ancora in corso, quindi non può fare altro a ciò che è scritto sopra
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("\n> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "remove", 6)) {                                                                                                             //Se il comando è "remove"
                            if (!analyzing) {                                                                                                                         //Verifica che non stia già analizzando
                                if ((strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL)) {                                                                 //Mentra analizza controlla se l'utente cambia setn o setm ed in tal caso verifica se sono correttamente inseriti
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");  //Stampa errore se sono stati inseriti comandi errati
                                    fflush(stdout);                                                                                                                   //Libera il buffer
                                } else if (checkArg(cmd, &argCounter)) {                                                                                              //Verifica gli argomenti inseriti a comando
                                    tempPath = malloc(argCounter * sizeof(char *));                                                                                   //Alloca memoria a tempPath
                                    for (j = 0; j < argCounter; j++) {                                                                                                //Cicla da 0 al numero di argomenti
                                        tempPath[j] = malloc(DIM_PATH * sizeof(char));                                                                                //Alloca a tempPath la dimensione DIM_PATH
                                    }                                                                                                                                 //
                                    strcpy(tempPath[0], strtok(cmd, " "));                                                                                            //Copia il comando cmd in tempPath[0] (cmd è il comando senza spazi)
                                    for (j = 1; j < argCounter; j++) {                                                                                                //Cicla da 0 al numero di argomenti
                                        strcpy(tempPath[j], strtok(NULL, " "));                                                                                       //Stessa copia della riga superirore
                                    }                                                                                                                                 //
                                    if ((parser2(argCounter, tempPath, lista, &count, &n, &m, &vReturn, &duplicate)) == 0) {                                          //Se il parsing funziona (=> i file sono stati aggiunti correttamente)

                                        if (vReturn > 0) {
                                            printf("\n" WHITE "Aggiunti" RESET " %d files\n", vReturn);
                                            if (duplicate > 0) {
                                                printf("%d files ignorati perche' gia' presenti\n\n", duplicate);
                                            } else {
                                                printf("\n");
                                            }

                                        } else {
                                            printf("\nNon sono stati aggiunti files");
                                            if (duplicate > 0) {
                                                printf("\n%d files ignorati perche' gia' presenti\n\n", duplicate);
                                            }
                                        }

                                        //Stampo il corretto parsing
                                    } else {  //
                                        //err_args_A();//
                                    }                                                   //
                                    for (j = 0; j < argCounter; j++) {                  //Cicla da 0 al numero di argomenti
                                        free(tempPath[j]);                              //Libera la lista tempPath[j]
                                    }                                                   //
                                    free(tempPath);                                     //Libera tempPath
                                } else {                                                //Altrimenti (se non ci sono setn e/o setm e non vengono inseriti comandi corretti)
                                    err_args_A();                                       //Ritorna errore degli argomenti di A
                                }                                                       //
                            } else {                                                    //Altrimenti l'analisi è ancora in corso, quindi non può fare altro a ciò che è scritto sopra
                                printf("Analisi in corso, comando non disponibile\n");  //
                            }                                                           //
                            printf("\n> ");                                             //
                            fflush(stdout);                                             //Libera il buffer
                        }                                                               //

                        if (!strncmp(cmd, "remove", 6)) {                                                                                                             //Se il comando è "remove"
                            if (!analyzing) {                                                                                                                         //Verifica che non stia già analizzando
                                if ((strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL)) {                                                                 //Mentra analizza controlla se l'utente cambia setn o setm ed in tal caso verifica se sono correttamente inseriti
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");  //Stampa errore se sono stati inseriti comandi errati
                                    fflush(stdout);
                                } else if (checkArg(cmd, &argCounter)) {                //Verifica gli argomenti inseriti a comando
                                    tempPath = malloc(argCounter * sizeof(char *));     //Alloca memoria a tempPath
                                    for (j = 0; j < argCounter; j++) {                  //Cicla da 0 al numero di argomenti
                                        tempPath[j] = malloc(DIM_PATH * sizeof(char));  //Alloca a tempPath la dimensione DIM_PATH
                                    }
                                    strcpy(tempPath[0], strtok(cmd, " "));       //Copia il comando cmd in tempPath[0] (cmd è il comando senza spazi)
                                    for (j = 1; j < argCounter; j++) {           //Cicla da 0 al numero di argomenti
                                        strcpy(tempPath[j], strtok(NULL, " "));  //Stessa copia della riga superirore
                                    }
                                    if ((parser2(argCounter, tempPath, lista, &count, &n, &m, &vReturn, &duplicate)) == 0) {  //Se il parsing funziona (=> i file sono stati aggiunti correttamente)
                                        cleanRemoved(lista);

                                        if (vReturn > 0) {
                                            printf(BOLDYELLOW "\n[ATTENTION]" RESET " Rimossi %d files\n", vReturn);
                                            if (duplicate > 0) {
                                                printf("%d files ignorati perche' non presenti\n\n", duplicate);
                                            } else {
                                                printf("\n");
                                            }
                                            if (lista->count == 0) {
                                                printf("La lista dei file e' vuota, per resettare il risultati precedenti usare" BOLDWHITE "reset" RESET "\n\n");  //
                                            } else {
                                                printf("Per analizzare anche i file che sono gia' stati analizzati usare il comando " BOLDWHITE "reanalyze" RESET "\n\n");  //
                                            }

                                        } else {
                                            printf(BOLDYELLOW "\n[ATTENTION]" RESET " Non sono stati rimossi file\n");
                                            printf("%d files ignorati perche' non presenti\n\n", duplicate);
                                        }

                                        //Stampo il corretto parsing
                                    } else {
                                        //err_args_A();
                                    }
                                    for (j = 0; j < argCounter; j++) {  //Libera tempPath
                                        free(tempPath[j]);
                                    }
                                    free(tempPath);
                                } else {
                                    err_args_A();  //Errore negli argomenti di A
                                }
                            } else {  //In alternativa vuol dire che sta analizzando e dunque non può rimuovere i files
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "reset", 5)) {                     //Se il comando e` reset
                            if (!analyzing) {                                //Controlla che il programma non sia in analisi
                                resetPathList(lista);                        //Svuota la lista
                                count = 0;                                   //Riporta il numero di file presenti nella lista
                                memset(sum, '\0', sizeof(char) * DIM_RESP);  //Setta la stringa dei totali a '\0
                                initialize_vector(v);                        //Azzera v
                                printf(BOLDYELLOW "\n[ATTENTION]" RESET " Tutti i file sono stati rimossi.\n\n");
                            } else {
                                printf("Analisi in corso, comando non disponibile\n");
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "reanalyze", 9)) {          //Se il comando e` reanalyze
                            if (!analyzing) {                         //Controlla che il sistema non sia in analisi
                                for (j = 0; j < lista->count; j++) {  //Setta tutti i file come non analizzati
                                    lista->analyzed[j] = 0;
                                }
                                pathSent = 0;  //Azzera il numero di percorsi da inviare
                                totFiles = 0;
                                notAnalyzed = 0;                                 //Azzera il numero di percorsi non analizzati
                                perc = 0;                                        //Azzerca il numero di percorsi ritornati
                                count = lista->count;                            //Setta count alla quantita` di percorsi inseriti nella lista
                                if (count > 0) {                                 //Se ci sono dei file da poter analizzare
                                    memset(sum, '\0', sizeof(char) * DIM_RESP);  //Setta la stringa dei totali a '\0
                                    initialize_vector(v);                        //Azzera i valori contenuti in v
                                    _write = FALSE;                              //Abilita la scrittura
                                    time(&start);                                //Inizio timer
                                    printf("\n");
                                } else {  //Messaggio nel caso di lista vuota
                                    printf(BOLDYELLOW "\n[ATTENTION]" RESET " Non ci sono file da analizzare\n\n> ");
                                    fflush(stdout);
                                }
                            } else {
                                printf("Analisi in corso, comando non disponibile\n> ");
                                fflush(stdout);
                            }
                        }

                        if (!strncmp(cmd, "analyze", 7)) {  //Se il comando inserito e` analyze
                            if (!analyzing) {               //Controlla che il sistema non sia in analisi
                                pathSent = 0;               //Azzera il numero di percorsi inviati
                                totFiles = 0;
                                notAnalyzed = 0;     //Azzera il numero di percorsi non analizzati
                                perc = 0;            //Azzera il numero di percorsi ritornati
                                if (count > 0) {     //Se ci sono dei file da poter analizzare
                                    _write = FALSE;  //Abilita la scrittura
                                    time(&start);
                                }  //Inizio timer
                                else
                                    printf("\nNon ci sono file da analizzare\n");
                            } else {
                                printf("\nAnalisi in corso, comando non disponibile\n");
                            }
                            printf("\n");
                        }

                        if (!strncmp(cmd, "clear", 5)) {  //Pulisce la console
                            if (!analyzing) {
                                system("clear");
                                printf("\n> ");
                                fflush(stdout);
                            } else {
                                printf(BOLDYELLOW "\n[ATTENZIONE]" RESET " Analisi in corso, non e` possibile pulire il terminale.\n");
                            }
                        }

                        if (!strncmp(cmd, "set", 3)) {  //Se il comando inserito e` set
                            if (checkArg(cmd, &argCounter)) {
                                if (argCounter == 2) {                              //Controlla che il numero di argomenti sia due
                                    if (!strncmp(cmd, "setn", 4)) {                 //Controlla che il comando sia setn
                                        dupl = strdup(cmd);                         //|
                                        new_n = strtok(dupl, " ");                  //|Rimuove gli spazi dal comando e ottiene il nuovo n
                                        new_n = strtok(NULL, " ");                  //|
                                        if (atoi(new_n) > 0 && atoi(new_n) != n) {  //Controlla che n sia > 0 e diverso dal precedente
                                            n = atoi(new_n);                        //Cambia il valore di n
                                            setOnFly(n, m, fd_1);                   //Avvia la procedura di setonfly
                                            printf("\n" BOLDYELLOW "[ATTENTION]" RESET " n e` stato modificato\n\n");
                                            if (analyzing) {      //Se il sistema sta analizzando
                                                i = 0;            //Riparte dal primo elemento di pathlist
                                                pathSent = perc;  //Porta il numero di percorsi inviati al numero di percorsi ricevuti
                                                _write = FALSE;   //Abilita la scrittura
                                            }
                                        } else {
                                            printf("\nn non e` stato modificato.\n");
                                            printf("Il valore inserito e` equivalente al precedente oppure e` uguale a 0\n\n");
                                        }
                                        free(dupl);  //Libera la memoria della stringa d'appoggio

                                    } else if (!strncmp(cmd, "setm", 4)) {          //Se il comando e` setm
                                        dupl = strdup(cmd);                         //|
                                        new_m = strtok(dupl, " ");                  //|Rimuove gli spazi dal comando e ottiene il nuovo m
                                        new_m = strtok(NULL, " ");                  //|
                                        if (atoi(new_m) > 0 && atoi(new_m) != m) {  //Controlla che m sia > 0 e diverso dal precedente
                                            m = atoi(new_m);                        //Aggiorna m
                                            setmOnFly(m, fd_1);                     //Avvia la procedura si setmOnFly
                                            printf("\n" BOLDYELLOW "[ATTENTION]" RESET " m e` stato modificato\n\n");
                                            if (analyzing) {      //Se il sistema sta analizzando
                                                i = 0;            //Riparte dal primo elemento di pathlist
                                                pathSent = perc;  //Porta il numero di percorsi inviati al numero di percorsi ricevuti
                                                _write = FALSE;   //Abilita la scrittura
                                            }
                                        } else {
                                            printf("\nm non e` stato modificato.\n");
                                            printf("Il valore inserito e` equivalente al precedente oppure e` uguale a 0\n\n");
                                        }
                                        free(dupl);  //Libera la memoria della stringa d'appoggio
                                    }
                                } else if (argCounter == 3) {                                  //Se il numero di argomenti e` 3
                                    if (strncmp(cmd, "setm", 4) && strncmp(cmd, "setn", 4)) {  //Controlla che non ci sia scritto sem o setn
                                        if (parseSetOnFly(cmd, &n, &m) == 0) {                 //Controlla che la correttezza del comando
                                            printf("\n" BOLDYELLOW "[ATTENTION]" RESET " n e m sono stati modificati\n\n");
                                            setOnFly(n, m, fd_1);  //Avvia la procedura di setOnFLy
                                            if (analyzing) {       //Se il sistema sta analizzando
                                                i = 0;             //Riparte dal primo elemento di pathlist
                                                pathSent = perc;   //Porta il numero di percorsi inviati al numero di percorsi ricevuti
                                                _write = FALSE;    //Abilita la scrittura
                                            }
                                        } else {
                                            printf("\nn e m non sono stati modificati.\n");
                                            printf("I valori inseriti sono equivalenti ai precedenti oppure uno dei due e` uguale a 0\n\n");
                                        }
                                    } else {
                                        printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");  //Stampa errore se sono stati inseriti comandi errati
                                        fflush(stdout);
                                    }
                                } else {
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");  //Stampa errore se sono stati inseriti comandi errati
                                    fflush(stdout);
                                }
                            }
                            printf("> ");
                            fflush(stdout);
                        }

                        if (!strncmp(cmd, "stat", 4)) {
                            if (!analyzing) {
                                printf(BOLDYELLOW "\n[ATTENTION]" RESET " Il comando " WHITE "stat" RESET " puo` essere utilizzato solo durante l'analisi.\n\n> ");
                                fflush(stdout);
                            } else {
                                printf(WHITE "Percentuale avanzamento" RESET ": %.3g%%\n\n", (float)perc / (float)totFiles * 100);
                            }
                        }
                    }
                }

                //R
                if (!_close && value_return == 0) {
                    if (retrieve) {                                                                                                                                                                                //Se R puo` richiedere dati
                        if (read(fd2_fifo, print_method, DIM_CMD) > 0) {                                                                                                                                           //Prova a leggere da R
                            if (!strncmp(print_method, "print", 5) || !strncmp(print_method, "-c", 2) || !strncmp(print_method, "-a", 2) || !strncmp(print_method, "-d", 2) || !strncmp(print_method, "-x", 2)) {  //Controlla che i comandi ricevuti siano corretti
                                retrieve = FALSE;                                                                                                                                                                  //Se si smette di leggere da R per inviargli i dati
                                if (analyzing) {                                                                                                                                                                   //Se sta analizzando stampa un messaggio di errore
                                    printf(BOLDYELLOW "\n[ATTENZIONE]" RESET " Analisi in corso, non e` possibile stampare\n");
                                }
                            }
                        }
                    } else {
                        if (!strncmp(print_method, "print", 5)) {     //Se il comando e` print
                            if (!analyzing) {                         //Controlla che il progrmma non sia in analisi
                                for (j = 0; j < lista->count; j++) {  //Invia tutti i percorsi inseriti
                                    write(fd1_fifo, lista->pathList[j], DIM_PATH);
                                }
                            } else {
                                strcpy(tmp_resp, "#ANALYZING");  //Se e` in analisi, setta la stringa di fine invio ad #ANALYZING per indicare che R non puo` ottenere dati in questo momento
                            }

                            write(fd1_fifo, tmp_resp, DIM_PATH);
                            strcpy(tmp_resp, "///");  //RIpristina la stringa di fine carattere a "///" nel caso in cui sia stata modificata precedentemente
                        }
                        if (!strncmp(print_method, "-d", 2)) {        //Se il comando e` -d
                            if (!analyzing) {                         //Se non sta analizzando
                                for (j = 0; j < lista->count; j++) {  //Invia tutti i percorsi inseriti con flag deleted
                                    if (lista->analyzed[j] == REMOVED) {
                                        write(fd1_fifo, lista->pathList[j], DIM_PATH);
                                    }
                                }
                            } else {
                                strcpy(tmp_resp, "#ANALYZING");  //In caso contrario avverte R che in questo momento non puo` ottenere datei
                            }

                            write(fd1_fifo, tmp_resp, DIM_PATH);
                            strcpy(tmp_resp, "///");  //RIpristina la stringa di fine carattere a "///" nel caso in cui sia stata modificata precedentemente
                        }
                        if (!strncmp(print_method, "-x", 2)) {        //Se il comando e` -x
                            if (!analyzing) {                         //Se non sta analizzando
                                for (j = 0; j < lista->count; j++) {  //Invia tutti i percorsi inseriti con flag analyzed
                                    if (lista->analyzed[j] == ANALYZED) {
                                        write(fd1_fifo, lista->pathList[j], DIM_PATH);
                                    }
                                }
                            } else {
                                strcpy(tmp_resp, "#ANALYZING");  //In caso contrario avverte R che in questo momento non puo` ottenere datei
                            }

                            write(fd1_fifo, tmp_resp, DIM_PATH);
                            strcpy(tmp_resp, "///");  //RIpristina la stringa di fine carattere a "///" nel caso in cui sia stata modificata precedentemente
                        }
                        if (!strncmp(print_method, "-c", 2)) {  //Se il comando e` -c
                            if (!analyzing) {                   //Se non sta analizzando
                                if (strstr(sum, ",") != NULL) {
                                    analyzeCluster(sum, type_resp);  //Esegue la statistica
                                } else {
                                    strcpy(type_resp, "#EMPTY");
                                }
                            } else {
                                strcpy(type_resp, "#ANALYZING");  //In caso contrario avverte R che in questo momento non puo` ottenere dati
                            }
                            write(fd1_fifo, type_resp, DIM_RESP);  //Scrive il messaggio
                        }
                        if (!strncmp(print_method, "-a", 2)) {    //Controlla se il comando e` -a
                            if (analyzing) {                      //Se sta analizzando
                                strcpy(type_resp, "#ANALYZING");  //Avverte R della situazione
                            } else {
                                if (strstr(sum, ",") != NULL) {
                                    strcpy(type_resp, sum);  //Copia nella stringa di risposta la somma dei valori
                                } else {
                                    strcpy(type_resp, "#EMPTY");
                                }
                            }
                            write(fd1_fifo, type_resp, DIM_RESP);  //Scrive la risposta
                        }
                        retrieve = TRUE;  //Ricomincia a leggere da R
                    }
                }

                //Quando WRITE e' in funzione inizia a mandare tutti i file con flag 0 di pathList
                if (!_write && value_return == 0) {                                    //Esegue il blocco finche` non ha finito di scrivere o non ci sono errori
                    analyzing = TRUE;                                                  //Va` in modalita` analisi
                    if (lista->analyzed[i] == 0) {                                     //Se il file i non e` stato analizzato
                        if (write(fd_1[WRITE], lista->pathList[i], DIM_PATH) == -1) {  //Prova a scrivere sulla pipe il percorso
                            if (errno != EAGAIN) {                                     //Se avviene un errore che non e` causato dalla dimensione della pipe
                                value_return = err_write();                            //Ritorna l'errore sulla scrittura
                            }
                        } else {
                            i++;         //Passa all'elemento successivo
                            pathSent++;  //Incrementa il numero di percorsi inviati
                            totFiles++;
                            if (pathSent == 1) {  //Se ha inviato il primo percorso
                                _read = FALSE;    //Abilita la read
                            }
                        }
                    } else {
                        i++;  //Passa all'elemento successivo
                    }
                    if (i == lista->count) {  //Qunado ha finito di inviare
                        _write = TRUE;        //Disabilita la write
                        i = 0;                //Azzera il contatore sulla posizione dei percorsi
                    }
                }

                //Read
                if (!_read && value_return == 0) {               //Esegue il blocco fiche` non c'e` piu` nulla nella pipe o non avviene un errore
                    if (read(fd_2[READ], resp, DIM_RESP) > 0) {  //Prova a leggere dal figlio
                        if (strstr(resp, "#") != NULL) {         //Controlla che ci sia almeno un # nel messaggio
                            tmp = strdup(resp);                  //Da qui esegue il parsing della stringa ritornatagli
                            id_r = atoi(strtok(tmp, "#"));
                            resp_val = strtok(NULL, "#");
                            tmpResp = strdup(resp_val);
                            firstVal = atoi(strtok(tmpResp, ","));
                            tmpPercorso = strdup(lista->pathList[id_r]);
                            file = strtok(tmpPercorso, "#");
                            file = strtok(NULL, "#");
                            if (firstVal != -1) {                                                   //Controlla che non ci siano stati errori nell'analisi
                                if (fileExist(file)) {                                              //File esistente
                                    lista->analyzed[id_r] = 1;                                      //Setta il flag ad Analizzato
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                    if (value_return == 0) {
                                        if (!compare_mtime(lista, id_r, file)) {  //Controlla che non sia stato modificato durante l'analisi
                                            stat(file, &attr);
                                            lista->last_edit[id_r] = attr.st_mtime;
                                            printf(BOLDYELLOW "[ATTENTION] " RESET "Il file %s\ne` stato modificato durante l'analisi.\nUsa il comando" BOLDWHITE " reanalyze" RESET " per rianalizzare tutti i file\n\n", file);
                                        }
                                    }
                                } else {
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    lista->analyzed[id_r] = 2;                                      //Setta il flag ad analizzato ma non piu` esistente
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                }
                            } else {                         //Caso in cui il file non e` piu' esistente
                                notAnalyzed++;               //Aumenta il numero di file non analizzati
                                lista->analyzed[id_r] = -1;  //Setta che il percoso non e` stato analizzato
                                perc++;                      //Aumenta il numero di file ricevuti
                            }

                            if (_write == TRUE && perc == totFiles && value_return == 0) {  //Se ha finito di scrivere e ha ricevuto tutti i percorsi
                                count -= lista->count;
                                time(&end);  //Diminuisce count della lunghezza della lista
                                elapsed = difftime(end, start);
                                printf(WHITE "Analizzati " RESET "%d " WHITE "files in %d secondi" RESET "\n", totFiles - notAnalyzed, (int)elapsed);
                                arrayToCsv(v, sum);  //Crea la stringa delle somme
                                pathSent = 0;        //Setta i percorsi inviati a 0
                                totFiles = 0;
                                analyzing = FALSE;  //Esce dalla procedura di analisi
                                _read = FALSE;      //Smette di leggere
                                if (notAnalyzed > 0) {
                                    printf(BOLDWHITE "[ATTENZIONE]" RESET " %d file non analizzati. Usare il comando " WHITE "print -d" RESET " visualizzare i file non piu\n", notAnalyzed);
                                }
                                count = notAnalyzed;
                                notAnalyzed = 0;
                                printf("\n> ");
                                fflush(stdout);
                            }

                            //Libera le variabili allocate
                            free(tmpPercorso);
                            free(tmp);
                            free(tmpResp);
                        }
                    }
                }
            }

            //Chiusura pipe anonime
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);

            //Chiusura pipe fifo
            if (value_return == 0) {
                if (close(fd1_fifo) == -1) {
                    value_return = err_close();
                }
                if (close(fd2_fifo) == -1) {
                    value_return = err_close();
                }
            }

            freePathList(lista);  //Libera la lista
        }
    }

    if (value_return == 0) {
        if (f == 0) {  //SON SIDE
            sprintf(n_exec, "%d", n);
            sprintf(m_exec, "%d", m);
            char *args[4] = {"./C", n_exec, m_exec, NULL};  //Costruisce la stringa degli argomenti per C

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