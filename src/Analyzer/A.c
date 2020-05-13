#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    //Interrupt initialize
    signal(SIGINT,handle_sigint);
    processes proc;

    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;
    
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    //node msg; //list used to pass path's to child
    //node filePath = NULL; //list of path's strings
    //parser variables
    int i; //Variabile usata per ciclare gli argomenti (argv[i])
    int value_return = 0; //Valore di ritorno
    int count = 0; //numero di file univoci da analizzare
    int perc = 0;
    
    array *lista = createPathList(10); //Nuova lista dei path

    char flag = FALSE; // se flag = true, non bisogna analizzare l'argomento. (l'argomento successivo è il numero o di n o di m)
    char setn = FALSE; // se setn = true, n è stato cambiato
    char setm = FALSE; // se setn = true, m è stato cambiato
    char errdir = FALSE;
    FILE *fp;
    char riga[1035];
    char resolved_path[PATH_MAX]; //contiene il percorso assoluto di un file

    //Variables for IPC
    int fd_1[2]; //Pipe
    int fd_2[2];
    pid_t f; //fork return value
    char array[7][20]; //Matrice di appoggio
    char* args[8]; //String og arguments to pass to child
    int _write = FALSE; //true when finish writing the pipe
    //int _read = FALSE; //true when fisnish reading from pipe

    value_return = parser(argc, argv, lista, &count, &n, &m);
    printf("argc:%d\n", argc);
    for(i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    printf("count:%d\nn:%d\nm:%d\n", count, n, m);
    
    if (value_return == 0){ //Esecuzione corretta
        printf("Numero file: %d,n=%d m=%d\n",count,n,m);
<<<<<<< HEAD
=======
        //printPathList(lista);
>>>>>>> parent of 2baa188... Feedback C
    }
    
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

    //Set Non-blocking pipes
    if(value_return == 0) {
        if(fcntl(fd_2[READ], F_SETFL, O_NONBLOCK)) { //Prova a sbloccare la pipe 2 in lettura
            value_return = err_fcntl(); //Se errore riporta il messaggio di errore
        }
        if(fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }
    
    if(value_return == 0) { //same as before
        f = fork(); //Fork dei processi
        if(f == -1) { //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork(); //in caso di errore setta il valore di ritorno a ERR_FORK
        }else{//insert process in list of OPEN processes
            proc.pid=f;
            proc.is_open="TRUE";
        }
    }

    //------------------------------------------------------------------------------

    if(value_return == 0) { //same
        if(f > 0) { //PARENT SIDE
            
            i = 0;
<<<<<<< HEAD
            while(value_return == 0 && (!_read || !_write)) { //cicla finche` non ha finito di leggere e scrivere
                //sleep(1);
=======
            while(value_return == 0 && (/*!_read || */!_write)) { //cicla finche` non ha finito di leggere e scrivere
                sleep(2);
>>>>>>> parent of 2baa188... Feedback C
                //Write
                if(!_write) {
                    for (i=0; i<count; i++){
                        if(write(fd_1[WRITE], lista->pathList[i], PATH_MAX) == -1) { //Prova a scrivere sulla pipe
                            value_return = err_write(); //Se fallisce da` errore
                            //ADD SIGNAL HANDLING
                        } else {
                            usleep(1000);
                        }
                    }
                    _write = TRUE;
                    
                    freePathList(lista);
                }

                //Read
<<<<<<< HEAD
                if(!_read) {
                    if(read(fd_2[READ], av, 2) > 1) {
                        perc++;
                        if(perc == count) {
                            _read = TRUE;
                        }
=======
                /*
                if(!_read) { //Non necessario presente solo per correttezza formale
                    if(read(fd_2[READ], char_count, 255) == 0) {
                        parse_string(char_count, v);
                        printf("%s\n",char_count);
                        i++;
                        if(i == count) _read = TRUE;
>>>>>>> parent of 2baa188... Feedback C
                    }
                }
                */
            }
            
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);
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
            //dup2(fd_2[WRITE], STDOUT_FILENO); DA ATTIVARE
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
    /*
    CONTROLLO I VARI PROCESSI CHE SIANO CHIUSI DAVVERO
    
    proc.is_open="FALSE";
    //DA IMPLEMENTARE: se il pid di C è ancora attivo allora va chiuso (in teoria non dovrebbe essere così ma mai dire mai)
    if (!strcmp(proc.is_open,"TRUE"))
    {
        printf("Attivo processo %d, va killato!!!",proc.pid);
    }*/

    return value_return;
}