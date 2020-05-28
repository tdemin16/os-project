#include "../lib/lib.h"

int value_return = 0;  //Valore di ritorno, globale per "send_to_R"
process *p;            //Dichiaro p (è globale perché handle_sigint non può avere parametri)
int fd1_fifo;          //A writes in R
int fd2_fifo;          //R writes in A

void handle_sigint(int sig) {                                                                                             //handler per il CTRL-C, ha l'obiettivo di
    printf(BOLDYELLOW "\n[ATTENZIONE]" RESET " Ricevuta terminazione per A, inizio terminazione processi C,P,Q ... \n");  //Stampo a terminale la corretta ricezione del comando Ctrl-C
    int i = p->count - 1;                                                                                                 //Parto dalla fine (poiché nella lista i processi figli vengono salvati dopo il processo padre)
    if (i > 0) {                                                                                                          //Se i > 0 => ci sono processi avviati
        while (i != 0) {                                                                                                  //Ciclo while fino a quando non ho controllato tutti i processi
            if (p->pid[i] > 0) {                                                                                          //Controllo che non sia un processo padre
                if (kill(p->pid[i], 9) == 0) {                                                                            //Provo a killare il pid[i]
                    printf(BOLDGREEN "\tProcesso %d terminato con successo!\n" RESET, p->pid[i]);                         //Se ha successo allora stampo la corretta terminazione
                } else {                                                                                                  //Altrimenti
                    printf(RED "\t[ERRORE]" RESET " Errore, non sono riuscito a chiudere il processo %d!", p->pid[i]);    //Qualcosa è andato storto nel kill
                }                                                                                                         //
            }                                                                                                             //
            i--;                                                                                                          //itero i--
        }                                                                                                                 //
    }                                                                                                                     //
    if (!close(fd1_fifo) && !close(fd2_fifo)) {                                                                           //close fifo
        printf(BOLDGREEN "[!]" RESET " Chiusura fifo completata\n");                                                      //
    }                                                                                                                     //
    freeList(p);                                                                                                          //Libero la lista di processi che ho salvato
    printf(BOLDGREEN "[COMPLETATO]" RESET " ... Chiusura processo terminata\n");                                          //Stampo a terminale la fine della chiusura processi
    exit(-1);                                                                                                             //Eseguo exit con codice di ritorno -1
}  //

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {  //handler per SIGTERM
    value_return = err_kill_process_A();                         //Nel caso accada ritorna errore processo A killato al di fuori del programma
}  //

void catch_sigterm() {                        //Funzione per consentire l'acquisizione del SIGTERM
    static struct sigaction _sigact;          //
                                              //
    memset(&_sigact, 0, sizeof(_sigact));     //
    _sigact.sa_sigaction = sig_term_handler;  //
    _sigact.sa_flags = SA_SIGINFO;            //
                                              //
    sigaction(SIGTERM, &_sigact, NULL);       //
}  //

int main(int argc, char *argv[]) {  //Main
    printf("Start A\n");            //Stampo l'esecuzione di A
    catch_sigterm();                //Avvio l'handler per SIGTERM
    signal(SIGINT, handle_sigint);  //Handler per SIGINT (Ctrl-C)
                                    //
    p = create_process(1);          //Allocate dynamically p with dimension 1

    //COMMUNICATION WITH R
    const char *fifo1 = "/tmp/A_to_R";  //Nome fifo con R
    const char *fifo2 = "/tmp/R_to_A";  //
    int retrieve = TRUE;                //
    int p_create = FALSE;               //
    char print_method[DIM_CMD];         //
    char type_resp[DIM_RESP];           //
    char tmp_resp[DIM_PATH];            //
    strcpy(tmp_resp, "///");            //

    //COMMUNICATION WITH M - STDIN
    char cmd[DIM_CMD];   //Comando rivevuto da M
    int _close = FALSE;  //
    char *new_n;         //
    char *new_m;         //
    char *dupl = NULL;   //

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;  //
    int m = 4;  //

    //Utility
    int i;            //Variabile usata per ciclare gli argomenti (argv[i])
    int j;            //
    int count = 0;    //numero di file univoci da analizzare
    int perc = 0;     //Ricevimento parziale file
    int oldperc = 0;  //Parziale precedente
    char n_exec[12];  //
    char m_exec[12];  //

    char analyzing = FALSE;    //
    int pathSent = 0;          //
    char *tmp = NULL;          //
    char *tmpResp = NULL;      //
    char *tmpPercorso = NULL;  //
    char **tempPath;           //
    int vReturn;               //
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
    initialize_vector(v);  //Inizializzazione vettore dei valori totali
    int val = 0;
    if (argc > 1) {                                                              //
        val = parser2(argc, argv, lista, &count, &n, &m, &vReturn, &duplicate);  //
        if (val == 0) {                                                          //Controlla i parametri passati ad A
            if (vReturn > 0) {                                                   //Vuol dire che c'è stato un errore nell'inserimento di PathList
                _write = FALSE;                                                  //Non può scrivere
            }                                                                    //
        } else {                                                                 //Altrimenti
            system("clear");                                                     //Libera il terminale
            printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n");                     //Avvisa l'inizio dell'Analyzer
            fflush(stdout);                                                      //Libera il buffer
            if (val == 1) err_args_A();                                          //Se val == 1 allora c'è errore argomenti di A non sufficienti
            printf("> ");                                                        //
            fflush(stdout);                                                      //
        }                                                                        //
    }                                                                            //

    insertProcess(p, getpid());  //Insert pid of A in process list

    //IPC
    if (value_return == 0) {            //Testo che non si siano verificati errori in precedenza
        if (pipe(fd_1) != 0) {          //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe();  //in caso di git errore setta il valore di ritorno a ERR_PIPE
        }                               //
    }                                   //
    if (value_return == 0) {            //Testo che non si siano verificati errori in precedenza
        if (pipe(fd_2) != 0) {          //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe();  //in caso di errore setta il valore di ritorno a ERR_PIPE
        }                               //
    }                                   //

    //Set Non-blocking pipes
    if (value_return == 0) {                             //
        if (fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) {    //Prova a sbloccare la pipe 1 in lettura
            value_return = err_fcntl();                  //Se errore riporta il messaggio di errore
        }                                                //
        if (fcntl(fd_1[WRITE], F_SETFL, O_NONBLOCK)) {   //Prova a sbloccare la pipe 1 in scrittura
            value_return = err_fcntl();                  //Se errore riporta il messaggio di errore
        }                                                //
        if (fcntl(fd_2[READ], F_SETFL, O_NONBLOCK)) {    //Prova a sbloccare la pipe 2 in lettura
            value_return = err_fcntl();                  //Se errore riporta il messaggio di errore
        }                                                //
        if (fcntl(fd_2[WRITE], F_SETFL, O_NONBLOCK)) {   //Prova a sbloccare la pipe 2 in scrittura
            value_return = err_fcntl();                  //Se errore riporta il messaggio di errore
        }                                                //
        if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {  //Prova a sbloccare lo stdin
            value_return = err_fcntl();                  //
        }                                                //
    }                                                    //

    //Open fifo in nonblocking read mode
    if (value_return == 0) {                                           //
        printf("Waiting for R...\n");                                  //
        printf("Use " BOLDYELLOW "[CTRL+C]" RESET " to interrupt\n");  //
        if (mkfifo(fifo1, 0666) == -1) {                               //Prova a creare la pipe
            if (errno != EEXIST) {                                     //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo();                             //Ritorna errore se l'operazione non va a buon fine
            }                                                          //
        }                                                              //

        fd1_fifo = open(fifo1, O_WRONLY);  //Prova ad aprire la pipe in scrittura
        if (fd1_fifo == -1) {              //Error handling
            value_return = err_fifo();     //value_return assume valore di errore della fifo
        }                                  //

        do {                                                //
            if (mkfifo(fifo2, 0666) == -1) {                //Prova a creare la pipe
                if (errno != EEXIST) {                      //In caso di errore controlla che la pipe non fosse gia` presente
                    value_return = err_fifo();              //Ritorna errore se l'operazione non va a buon fine
                }                                           //
            }                                               //
            fd2_fifo = open(fifo2, O_RDONLY | O_NONBLOCK);  //Apre la fifo2 in modalità solo lettura
            if (fd2_fifo != -1) {                           //Se diverso da -1
                p_create = TRUE;                            //Allora p_create = true
            } else if (errno != ENOENT) {                   //Altrimenti se diverso da ENOENT
                value_return = err_fifo();                  //value_return prende errore fifo
            }                                               //
        } while (value_return == 0 && !p_create);           //Cicla fino a quando value_return == 0 e p_create è falso
    }                                                       //

    system("clear");                                    //pulisce il terminale
    printf(BOLDWHITE "ANALYZER AVVIATO" RESET "\n\n");  //Avvisa l'avvio di analyzer
    fflush(stdout);                                     //libera il buffer

    if (value_return == 0 && !_close) {  //Se non ci sono stati errori e il flag close non è a true (_close serve per la chiusura dei programmi)
        f = fork();                      //Fork dei processi
        if (f == -1) {                   //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork();   //in caso di errore setta il valore di ritorno a ERR_FORK
        }                                //
    }                                    //

    //------------------------------------------------------------------------------

    if (value_return == 0 && !_close) {
        if (f > 0) {                            //PARENT SIDE
            insertProcess(p, f);                //Inserisce processo figlio f nella lista processi p
            i = 0;                              //
            char str[15];                       //
            sprintf(str, "A%d.txt", getpid());  //
            FILE *debug = fopen(str, "a");      //
            fprintf(debug, "AVVIATO A\n");      //
            fclose(debug);                      //
            printf("> ");                       //
            fflush(stdout);

            while (value_return == 0 && !_close) {  //cicla finche` non ha finito di leggere e scrivere o avviene un errore

                //M - STDIN
                if (!_close) {                                   //Controlla close non sia già settato a true
                    if (read(STDIN_FILENO, cmd, DIM_CMD) > 0) {  //La read non ha errori
                        if (!strncmp(cmd, "close", 5)) {         //Verifica se cmd è close, in tal caso prosegue alla chiusura dei processi
                            debug = fopen(str, "a");             //
                            fprintf(debug, "A: %s\n", cmd);      //
                            fclose(debug);                       //
                            closeAll(fd_1);                      //chiudi tutti i processi figli

                            while (wait(NULL) > 0)                         //
                                ;                                          //
                            _close = TRUE;                                 //flag close settato a TRUE
                            printf(BOLDWHITE "A" RESET ": Closing...\n");  //
                        }                                                  //

                        if (!strncmp(cmd, "add", 3)) {                                                                                                                //Se invece il comando è "add"
                            if (!analyzing) {                                                                                                                         //Verifica che non stia già analizzando
                                debug = fopen(str, "a");                                                                                                              //
                                fprintf(debug, "A: %s\n", cmd);                                                                                                       //
                                fclose(debug);                                                                                                                        //
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
                                debug = fopen(str, "a");                                                                                                              //
                                fprintf(debug, "A: %s\n", cmd);                                                                                                       //
                                fclose(debug);                                                                                                                        //
                                if ((strstr(cmd, "-setn") != NULL || strstr(cmd, "-setm") != NULL)) {                                                                 //Mentra analizza controlla se l'utente cambia setn o setm ed in tal caso verifica se sono correttamente inseriti
                                    printf(BOLDRED "\n[ERRORE] " RESET "Comando inserito non corretto.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");  //Stampa errore se sono stati inseriti comandi errati
                                    fflush(stdout);                                                                                                                   //
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
                                    } else {  //
                                        //err_args_A();
                                    }
                                    for (j = 0; j < argCounter; j++) {  //
                                        //printf("ARG[%d] - %s\n",j,tempPath[j]);
                                        free(tempPath[j]);                              //libera tempPath[j]
                                    }                                                   //
                                    free(tempPath);                                     //
                                } else {                                                //Altrimenti
                                    err_args_A();                                       //Errore negli argomenti di A
                                }                                                       //
                            } else {                                                    //In alternativa vuol dire che sta analizzando e dunque non può rimuovere i files
                                printf("Analisi in corso, comando non disponibile\n");  //
                            }                                                           //
                            printf("> ");                                               //
                            fflush(stdout);                                             //Libera il buffer
                        }                                                               //

                        if (!strncmp(cmd, "reset", 5)) {                                                           //
                            if (!analyzing) {                                                                      //
                                debug = fopen(str, "a");                                                           //
                                fprintf(debug, "A: %s\n", cmd);                                                    //
                                fclose(debug);                                                                     //
                                resetPathList(lista);                                                              //
                                count = 0;                                                                         //
                                memset(sum, '\0', sizeof(char) * DIM_RESP);                                        //
                                initialize_vector(v);                                                              //
                                printf(BOLDYELLOW "\n[ATTENTION]" RESET " Tutti i file sono stati rimossi.\n\n");  //
                            } else {                                                                               //
                                printf("Analisi in corso, comando non disponibile\n");                             //
                            }                                                                                      //
                            printf("> ");                                                                          //
                            fflush(stdout);                                                                        //
                        }                                                                                          //

                        if (!strncmp(cmd, "debug", 5)) {                           //
                            printf("count = %d\npathSent=%d\n", count, pathSent);  //
                            printf("> ");                                          //
                            fflush(stdout);                                        //
                        }                                                          //

                        if (!strncmp(cmd, "reanalyze", 9)) {                                                           //
                            if (!analyzing) {                                                                          //
                                debug = fopen(str, "a");                                                               //
                                fprintf(debug, "A: %s\n", cmd);                                                        //
                                fclose(debug);                                                                         //
                                for (j = 0; j < lista->count; j++) {                                                   //
                                    lista->analyzed[j] = 0;                                                            //
                                }                                                                                      //
                                pathSent = 0;                                                                          //
                                notAnalyzed = 0;                                                                       //
                                perc = 0;                                                                              //
                                count = lista->count;                                                                  //
                                if (count > 0) {                                                                       //
                                    memset(sum, '\0', sizeof(char) * DIM_RESP);                                        //
                                    initialize_vector(v);                                                              //
                                    _write = FALSE;                                                                    //
                                    printf("\n");                                                                      //
                                } else {                                                                               //
                                    printf(BOLDYELLOW "\n[ATTENTION]" RESET " Non ci sono file da analizzare\n\n> ");  //
                                    fflush(stdout);                                                                    //
                                }                                                                                      //
                            } else {                                                                                   //
                                printf("Analisi in corso, comando non disponibile\n> ");                               //
                                fflush(stdout);                                                                        //
                            }                                                                                          //
                        }                                                                                              //

                        if (!strncmp(cmd, "analyze", 7)) {                                //
                            if (!analyzing) {                                             //
                                debug = fopen(str, "a");                                  //
                                fprintf(debug, "A: %s\n", cmd);                           //
                                fclose(debug);                                            //
                                pathSent = 0;                                             //
                                notAnalyzed = 0;                                          //
                                perc = 0;                                                 //
                                if (count > 0)                                            //
                                    _write = FALSE;                                       //
                                else                                                      //
                                    printf("\nNon ci sono file da analizzare\n");         //
                            } else {                                                      //
                                printf("\nAnalisi in corso, comando non disponibile\n");  //
                            }                                                             //
                            printf("\n");                                                 //
                        }                                                                 //

                        if (!strncmp(cmd, "oldprint", 8)) {  //
                            printPathList(lista);            //
                            printf("\n> ");                  //
                            fflush(stdout);                  //
                        }                                    //

                        if (!strncmp(cmd, "clear", 5)) {  //
                            system("clear");              //
                            printf("\n> ");               //
                            fflush(stdout);               //
                        }                                 //

                        if (!strncmp(cmd, "set", 3)) {                                                                           //
                            debug = fopen(str, "a");                                                                             //
                            fprintf(debug, "A: %s\n", cmd);                                                                      //
                            fclose(debug);                                                                                       //
                            if (checkArg(cmd, &argCounter)) {                                                                    //
                                if (argCounter == 2) {                                                                           //
                                    if (!strncmp(cmd, "setn", 4)) {                                                              //
                                        dupl = strdup(cmd);                                                                      //
                                        new_n = strtok(dupl, " ");                                                               //
                                        new_n = strtok(NULL, " ");                                                               //
                                        if (atoi(new_n) > 0 && atof(new_n) != n) {                                               //
                                            n = atoi(new_n);                                                                     //
                                            setOnFly(n, m, fd_1);                                                                //
                                            printf("\n" BOLDYELLOW "[ATTENTION]" RESET " n e` stato modificato\n\n");            //
                                            if (analyzing) {                                                                     //
                                                i = 0;                                                                           //
                                                pathSent = perc;                                                                 //
                                                _write = FALSE;                                                                  //
                                            }                                                                                    //
                                        } else {                                                                                 //
                                            printf("\nn non e` stato modificato.\n");                                            //
                                            printf("Il valore inserito e` equivalente al precedente oppure e` uguale a 0\n\n");  //
                                        }                                                                                        //
                                        free(dupl);                                                                              //

                                    } else if (!strncmp(cmd, "setm", 4)) {                                                                //
                                        dupl = strdup(cmd);                                                                               //
                                        new_m = strtok(dupl, " ");                                                                        //
                                        new_m = strtok(NULL, " ");                                                                        //
                                        if (atoi(new_m) > 0) {                                                                            //
                                            m = atoi(new_m);                                                                              //
                                            setmOnFly(m, fd_1);                                                                           //
                                            printf("\n" BOLDYELLOW "[ATTENTION]" RESET " m e` stato modificato\n\n");                     //
                                            if (analyzing) {                                                                              //
                                                i = 0;                                                                                    //
                                                pathSent = perc;                                                                          //
                                                _write = FALSE;                                                                           //
                                            }                                                                                             //
                                        } else {                                                                                          //
                                            printf("\nm non e` stato modificato.\n");                                                     //
                                            printf("Il valore inserito e` equivalente al precedente oppure e` uguale a 0\n\n");           //
                                        }                                                                                                 //
                                        free(dupl);                                                                                       //
                                    }                                                                                                     //
                                } else if (argCounter == 3) {                                                                             //
                                    if (parseSetOnFly(cmd, &n, &m) == 0) {                                                                //
                                        printf("\n" BOLDYELLOW "[ATTENTION]" RESET " n e m sono stati modificati\n\n");                   //
                                        setOnFly(n, m, fd_1);                                                                             //
                                        if (analyzing) {                                                                                  //
                                            i = 0;                                                                                        //
                                            pathSent = perc;                                                                              //
                                            _write = FALSE;                                                                               //
                                        }                                                                                                 //
                                    } else {                                                                                              //
                                        printf("\nn e m non sono stati modificati.\n");                                                   //
                                        printf("I valori inseriti sono equivalenti ai precedenti oppure uno dei due e` uguale a 0\n\n");  //
                                    }                                                                                                     //
                                }                                                                                                         //
                            }                                                                                                             //
                            printf("> ");                                                                                                 //
                            fflush(stdout);                                                                                               //
                        }                                                                                                                 //
                    }                                                                                                                     //
                }                                                                                                                         //

                //R//
                if (!_close && value_return == 0) {                                                                                          //
                    if (retrieve) {                                                                                                          //Try read from R//
                        if (read(fd2_fifo, print_method, DIM_CMD) > 0) {                                                                     //
                            if (!strncmp(print_method, "print", 5) || !strncmp(print_method, "-c", 2) || !strncmp(print_method, "-a", 2)) {  //
                                retrieve = FALSE;                                                                                            //
                                if (analyzing) {                                                                                             //
                                    printf(BOLDYELLOW "\n[ATTENZIONE]" RESET " Analisi in corso, non e` possibile stampare\n");              //
                                }                                                                                                            //
                            }                                                                                                                //
                        }
                    } else {                                                                //
                        if (!strncmp(print_method, "print", 5)) {                           //
                            if (!analyzing) {                                               //
                                if (lista->count > 0) {                                     //
                                    for (j = 0; j < lista->count; j++) {                    //
                                        write(fd1_fifo, lista->pathList[j], DIM_PATH + 2);  //
                                    }                                                       //
                                }                                                           //
                            } else {                                                        //
                                strcpy(tmp_resp, "#ANALYZING");                             //
                            }                                                               //

                            write(fd1_fifo, tmp_resp, DIM_PATH + 2);   //
                            strcpy(tmp_resp, "///");                   //
                        }                                              //
                        if (!strncmp(print_method, "-c", 2)) {         //
                            if (!analyzing) {                          //
                                analyzeCluster(sum, type_resp);        //
                            } else {                                   //
                                strcpy(type_resp, "#ANALYZING");       //
                            }                                          //
                            write(fd1_fifo, type_resp, DIM_RESP);      //
                        }                                              //
                        if (!strncmp(print_method, "-a", 2)) {         //
                            if (analyzing) strcpy(sum, "#ANALYZING");  //
                            write(fd1_fifo, sum, DIM_RESP);            //
                        }                                              //
                        retrieve = TRUE;                               //
                    }                                                  //
                }                                                      //

                //Quando WRITE e' in funzione inizia a mandare tutti i file con flag 0 di pathList//
                if (!_write && value_return == 0) {                                    //Esegue il blocco finche` non ha finito di scrivere//
                    analyzing = TRUE;                                                  //
                    if (lista->analyzed[i] == 0) {                                     //
                        if (write(fd_1[WRITE], lista->pathList[i], DIM_PATH) == -1) {  //Prova a scrivere sulla pipe//
                            if (errno != EAGAIN) {                                     //Se avviene un errore e non e` causato dalla dimensione della pipe//
                                value_return = err_write();                            //Ritorna l'errore sulla scrittura//
                            }                                                          //
                        } else {                                                       //
                            debug = fopen(str, "a");                                   //
                            fprintf(debug, "A: INVIATO %s\n", lista->pathList[i]);     //
                            fclose(debug);                                             //
                            i++;                                                       //
                            pathSent++;                                                //
                            if (pathSent == 1) {                                       //
                                _read = FALSE;                                         //
                                debug = fopen(str, "a");                               //
                                fprintf(debug, "A: ABILITO LA READ\n");                //
                                fclose(debug);                                         //
                            }                                                          //
                        }                                                              //
                    } else {                                                           //
                        i++;                                                           //
                    }                                                                  //
                    if (i == lista->count) {                                           //Qunado ha finito di inviare//
                        _write = TRUE;                                                 //Setta il flag a true//
                        i = 0;                                                         //
                        debug = fopen(str, "a");                                       //
                        fprintf(debug, "A: CHIUDO LA READ\n");                         //
                        fclose(debug);                                                 //
                    }                                                                  //
                }                                                                      //

                //Read
                if (!_read && value_return == 0) {               //Esegue il blocco fiche` non c'e` piu` nulla nella pipe
                    if (read(fd_2[READ], resp, DIM_RESP) > 0) {  //Pero` potremmo vedere se sto controllo serve realmente

                        if (strstr(resp, "#") != NULL) {            //Controlla che ci sia almeno un # nel messaggio
                            tmp = strdup(resp);                     //
                            id_r = atoi(strtok(tmp, "#"));          //id del file da valutare
                            resp_val = strtok(NULL, "#");           //valori
                            tmpResp = strdup(resp_val);             //
                            firstVal = atoi(strtok(tmpResp, ","));  //primo valore
                            tmpPercorso = strdup(lista->pathList[id_r]);
                            file = strtok(tmpPercorso, "#");                                        //Recupera path corrispondente nella lista
                            file = strtok(NULL, "#");                                               //percorso
                            debug = fopen(str, "a");                                                //
                            fprintf(debug, "A: RICEVUTO %s\n", lista->pathList[id_r]);              //
                            fclose(debug);                                                          //
                            if (firstVal != -1) {                                                   //
                                if (fileExist(file)) {                                              // File esistente
                                    lista->analyzed[id_r] = 1;                                      //Setta il flag ad Analizzato
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                    if (value_return == 0) {                                        //
                                        if (!compare_mtime(lista, id_r, file)) {                    //
                                            printf("\nIl file %s\ne` stato modificato durante l'analisi.\nUsa il comando" BOLDWHITE " reanalyze" RESET " per rianalizzarlo\n\n", file);
                                        }                                                           //
                                    }                                                               //
                                } else {                                                            //
                                    if (addCsvToArray(resp_val, v)) value_return = err_overflow();  //Aggiunge il file al vettore delle somme
                                    lista->analyzed[id_r] = 2;                                      //Setta il flag ad analizzato ma non piu` esistente
                                    perc++;                                                         //Aumenta l'avanzamento della barretta
                                }
                            } else {                         //Caso in cui il file non e` piu' esistente
                                notAnalyzed++;               //
                                lista->analyzed[id_r] = -1;  //
                                perc++;                      //
                            }                                //

                            //Barretta

                            /* char *ptr = strchr(lista->pathList[id_r], '\0');
                            if (ptr) {
                                int index = ptr - lista->pathList[id_r];
                                char *last_ten = &lista->pathList[id_r][index - 50];
                                printf("\033[A\33[2KT\r"BOLDGREEN"[ANALYZED]"RESET" ..%s\n", last_ten);
                                fflush(stdout);
                            } */

                            if ((int)((float)perc * 10 / (float)pathSent) > oldperc && value_return == 0) {  //
                                oldperc = (int)((float)perc * 10 / (float)pathSent);                         //
                                //system("clear");
                                //percAvanzamento(perc, count);
                            }

                            if (_write == TRUE && perc == pathSent && value_return == 0) {            //
                                count -= lista->count;                                                //
                                printf(WHITE "Numero file analizzati" RESET ": %d\n\n> ", pathSent);  //
                                fflush(stdout);                                                       //
                                arrayToCsv(v, sum);                                                   //
                                //printStat_Cluster(sum);
                                //printf("\n> ");
                                //fflush(stdout);
                                //setOnFly(4,5,fd_1);
                                //sleep(5);
                                //closeAll(fd_1);
                                //_close = TRUE;
                                pathSent = 0;       //
                                analyzing = FALSE;  //
                                _read = FALSE;      //
                            }                       //

                            free(tmpPercorso);  //
                            free(tmp);          //
                            free(tmpResp);      //
                        }                       //
                    }                           //
                }                               //
            }                                   //

            //Chiusura pipe
            close(fd_1[READ]);   //
            close(fd_1[WRITE]);  //
            close(fd_2[READ]);   //
            close(fd_2[WRITE]);  //

            //Chiusura fifo
            if (value_return == 0) {             //
                if (close(fd1_fifo) == -1) {     //
                    value_return = err_close();  //
                }                                //
                if (close(fd2_fifo) == -1) {     //
                    value_return = err_close();  //
                }                                //
            }                                    //
                                                 //
            freePathList(lista);                 //
        }                                        //
    }                                            //

    if (value_return == 0) {                                //
        if (f == 0) {                                       //SON SIDE
            sprintf(n_exec, "%d", n);                       //
            sprintf(m_exec, "%d", m);                       //
            char *args[4] = {"./C", n_exec, m_exec, NULL};  //
            //Redirects pipes to STDIN and STDOUT
            dup2(fd_1[READ], STDIN_FILENO);    //
            dup2(fd_2[WRITE], STDOUT_FILENO);  //
            //Closing pipes
            close(fd_1[READ]);                   //
            close(fd_1[WRITE]);                  //
            close(fd_2[READ]);                   //
            close(fd_2[WRITE]);                  //
            if (execvp(args[0], args) == -1) {   //Test exec
                value_return = err_exec(errno);  //Set value return
            }
            /*
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
            */
        }  //
    }      //

    freeList(p);  //

    return value_return;  //
}  //