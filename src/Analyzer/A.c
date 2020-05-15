#include "../lib/lib.h"

int main(int argc, char *argv[]) {

    int value_return = 0; //Valore di ritorno, globale per "send_to_R"

    //Interrupt initialize
    //signal(SIGINT,handle_sigint);
    struct stat sb;
    //initialize_processes(proc,4);

    //COMMUNICATION WITH R
    const char* fifo = "/tmp/A_R_Comm";
    pid_t fd_fifo;
    
    //printf("Processo: %d, is %s\n",proc[0].pid,proc[0].folder);

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;
    
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    //node msg; //list used to pass path's to child
    //node filePath = NULL; //list of path's strings
    //parser variables
    int i; //Variabile usata per ciclare gli argomenti (argv[i])
    int count = 0; //numero di file univoci da analizzare
    int perc = 0;

    array *lista = createPathList(10); //Nuova lista dei path

    //Variables for IPC
    int fd_1[2]; //Pipe
    int fd_2[2];
    pid_t f; //fork return value
    char array[7][20]; //Matrice di appoggio
    char* args[8]; //String og arguments to pass to child
    int _write = FALSE; //true when finish writing the pipe
    int _read = FALSE; //true when fisnish reading from pipe
    char ad[2];

    value_return = parser(argc, argv, lista, &count, &n, &m);
    
    if (value_return == 0){ //Esecuzione corretta
        printf("Numero file: %d,n=%d m=%d\n",count,n,m);
        //printPathList(lista);


        //proc = malloc(2*(n*m) + 2); //malloc proc with n-P process and m-Q process doubled for son's + 2 for C (...)
        //initialize_processes(proc,((2*n*m) + 2));
    }

    //proc[p_dim] = getpid(); //Aggiungo PID del padre
    //p_dim++;                //Aumento dimensione
    
    //IPC
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_1) != 0) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di git errore setta il valore di ritorno a ERR_PIPE
        }
    }
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_2) != 0) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di errore setta il valore di ritorno a ERR_PIPE
        }
    }
    if(value_return == 0) {
        if(mkfifo(fifo, 0666) == -1) { //Prova a creare la pipe
            if(errno != EEXIST) { //In caso di errore controlla che la pipe non fosse gia` presente
                value_return = err_fifo(); //Ritorna errore se l'operazione non va a buon fine
            }
        }
    }

    //Set Non-blocking pipes
    if(value_return == 0) {
        if(fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) { //Prova a sbloccare la pipe 1 in lettura
            value_return = err_fcntl(); //Se errore riporta il messaggio di errore
        }
        if(fcntl(fd_1[WRITE], F_SETFL, O_NONBLOCK)) { //Prova a sbloccare la pipe 1 in scrittura
            value_return = err_fcntl(); //Se errore riporta il messaggio di errore
        }
        if(fcntl(fd_2[READ], F_SETFL, O_NONBLOCK)) { //Prova a sbloccare la pipe 2 in lettura
            value_return = err_fcntl(); //Se errore riporta il messaggio di errore
        }
        if(fcntl(fd_2[WRITE], F_SETFL, O_NONBLOCK)) { //Prova a sbloccare la pipe 2 in scrittura
            value_return = err_fcntl(); //Se errore riporta il messaggio di errore
        }
        if(fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) { //Prova a sbloccare lo stdin
            value_return = err_fcntl();
        }
    }
    
    //Open fifo in nonblocking read mode
    if(value_return == 0) {
        fd_fifo = open(fifo, O_RDONLY | O_NONBLOCK); //Prova ad aprire la pipe in scrittura
        if(fd_fifo == -1) { //Error handling
            value_return = err_file_open(); //Errore nell'apertura del file
        }
    }

    if(value_return == 0) { //same as before
        f = fork(); //Fork dei processi
        if(f == -1) { //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork(); //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    //------------------------------------------------------------------------------

    if(value_return == 0) { //same
        if(f > 0) { //PARENT SIDE
        //proc[p_dim] = f;//These 2 lines are adding fork() process 
        //p_dim++;        //Increase the dimension
            i = 0;
            while(value_return == 0 && (!_read || !_write)) { //cicla finche` non ha finito di leggere e scrivere
                //sleep(2);
                //Write
                if(!_write) {
                    if(write(fd_1[WRITE], lista->pathList[i], PATH_MAX) == -1) { //Prova a scrivere sulla pipe
                        if (errno != EAGAIN){
                            value_return = err_write();
                        } else {
                            //fprintf(stderr,"A->C: Pipe piena\n");
                        }
                        //Se fallisce da` errore
                        //ADD SIGNAL HANDLING
                    } else {
                        i++;
                        //fprintf(stderr,"A->C: scrivo\n");
                        if(i == count) {
                            _write = TRUE;
                            freePathList(lista);
                            //fprintf(stderr,"######## A->C: Scritto tutto\n");
                        }
                    }
                }
                
                //Read
                //fprintf(stderr,"A<-C: leggo\n");
                if(!_read) {
                    if(read(fd_2[READ], ad, 2) > 1) {
                        printf("%s", ad);
                        perc++;
                        if(perc == count*m) {
                            _read = TRUE;
                            printf("\n");
                        }
                    }
                }
            }
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);
            
            if(value_return == 0) {
                if(close(fd_fifo) == -1) {
                    value_return = err_close();
                }
            }

            if(value_return == 0) {
                if(unlink(fifo) == -1) {
                    value_return = err_unlink();
                }
            }
        }
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE

            //Creates char* args []
            strcpy(array[0], "./C");
            strcpy(array[1], "-nfiles");
            sprintf(array[2], "%d", count);
            strcpy(array[3], "-setn");
            sprintf(array[4], "%d", n);
            strcpy(array[5], "-setm");
            sprintf(array[6], "%d", m);
            //Copy into args
            for(i = 0; i < 7; i++) {
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
            if(execvp(args[0], args) == -1) { //Test exec
                value_return = err_exec(errno); //Set value return
            }
        }
    }
    
    return value_return;
}