#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;
    
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    //node msg; //list used to pass path's to child
    //node filePath = NULL; //list of path's strings
    //parser variables
    int i; //Variabile usata per ciclare gli argomenti (argv[i])
    int j;
    int value_return = 0; //Valore di ritorno
    int count = 0; //numero di file univoci da analizzare
    int perc = 0;
    

    array *lista = createPathList(10); //Nuova lista dei path

    char resolved_path[PATH_MAX]; //contiene il percorso assoluto di un file
    char *tmp;
    char flag = FALSE; // se flag = true, non bisogna analizzare l'argomento. (l'argomento successivo è il numero o di n o di m)
    char setn = FALSE; // se setn = true, n è stato cambiato
    char setm = FALSE; // se setn = true, m è stato cambiato
    char errdir = FALSE;
    FILE *fp;
    char riga[1035];

    //Variables for IPC
    int fd_1[2]; //Pipe
    int fd_2[2];
    pid_t f; //fork return value
    char array[7][20]; //Matrice di appoggio
    char* args[8]; //String og arguments to pass to child
    int _write = FALSE; //true when finish writing the pipe
    int _read = FALSE; //true when fisnish reading from pipe
    char av[2];

    if(argc < 1) { //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_A();
    }
    else {
        
        for(i = 1; i < argc && value_return == 0; i++) {
            //printf("Argv: %s\n",argv[i]);
            if(!strcmp(argv[i], "-setn")) {//----ERRORI -setn
                if (i+1<argc){ //controlla che ci sia effettivamente un argomento dopo il -setn
                    n = atoi(argv[i + 1]);
                    if(n == 0) value_return = err_args_A(); //Il campo dopo -setn non è un numero
                    if(setn == TRUE) value_return = err_args_A();   //n gia' settato
                    flag = TRUE; //Salto prossimo argomento
                    setn = TRUE; //n e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    value_return = err_args_A();
                }
                
            }
            else if(!strcmp(argv[i], "-setm")) {//-----ERRORI -setm
                if (i+1<argc){ //controlla che ci sia effettivamente un argomento dopo il -setn
                    m = atoi(argv[i+1]);
                    if(m == 0) value_return = err_args_A(); //Il campo dopo -setm non è un numero
                    if(setm == TRUE) value_return = err_args_A(); //m gia' settato
                    flag = TRUE;    //Salto il prossimo argomento
                    setm = TRUE;    //m e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    value_return = err_args_A();
                }


            }//else if(strncmp(argv[i], "-", 1) == 0){//-----ERRORI input strani che iniziano con -
            //    value_return = err_args_A();
            //}

            if (flag == FALSE && value_return == 0){ //------Vuol dire che argv[i] e' un file o una cartella
                /*  Viene utilizzato il comando:  test -d [dir] && find [dir] -type f -follow -print || echo "-[ERROR]"
                    Il comando test -d controlla l'esistenza del file/directory input. In caso di successo viene lanciato find
                    In caso di successo viene lanciato find che restituisce la lista di tutti i file nella cartella e nelle sottocartelle
                    Se l'input non esiste restituisce -[ERROR], in modo che possa essere intercettato dal parser
                */
               
                char command[strlen("(test -f  || test -d ) && find ") + strlen(argv[i])*3 + strlen(" -type f -follow -print || echo \"-[ERROR]\"")+ 1]; //Creazione comando
                strcpy(command, "(test -f ");
                strcat(command, argv[i]);
                strcat(command, " || test -d ");
                strcat(command, argv[i]);
                strcat(command, ") && find ");
                strcat(command, argv[i]);
                strcat(command, " -type f -follow -print || echo \"-[ERROR]\"");
                //printf("%s\n",command);
                fp = popen(command, "r"); //avvia il comando e in fp prende l'output
                if (fp == NULL) //Se il comando non va a buon fine
                {
                    value_return = err_args_A();
                } else { //Il comando va a buon fine
                    while (fgets(riga, sizeof(riga), fp) != NULL && errdir == FALSE) //Legge riga per riga e aggiunge alla lista
                {
                    if (strcmp(riga,"-[ERROR]\n")){
                        //memset( resolved_path, '\0', sizeof(resolved_path));
                        realpath(riga, resolved_path);  //risalgo al percorso assoluto
                        resolved_path[strlen(resolved_path)-1] = 0; //tolgo l'ultimo carattere che manderebbe a capo                             
                        if (insertPathList(lista, resolved_path)){
                            count++;
                            //printf("%s\n", resolved_path);
                        }
                        
                        
                    } else { //Intercetta l'errore riguardante file o cartelle non esistenti
                        errdir = TRUE; //Metto il flag errore file/directory sbagliati
                    }
                }
                pclose(fp); //chiudo fp
                if (errdir == TRUE) value_return = err_input_A(argv[i]); //Mando l'errore per la directory
                }
            } else {
                flag = FALSE; //Analisi argomento saltata, rimetto flag a false
            }
        }
        if(count == 0 && value_return == 0) value_return = err_args_A(); //counter is higher than zero, if not gives an error (value_return used to avoid double messages)
    }
    
    if (value_return == 0){ //Esecuzione corretta
        //system("clear");
        printf("Numero file: %d,n=%d m=%d\n",count,n,m);
        //printf("[..........]\n");
        printPathList(lista);
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
        }
    }

    //------------------------------------------------------------------------------

    if(value_return == 0) { //same
        if(f > 0) { //PARENT SIDE
            
            i = 0;
            while(value_return == 0 && (!_read || !_write)) { //cicla finche` non ha finito di leggere e scrivere
                //sleep(1);
                //Write
                if(!_write) {
                    if(write(fd_1[WRITE], lista->pathList[i], PATH_MAX) == -1) { //Prova a scrivere sulla pipe
                        value_return = err_write(); //Se fallisce da` errore
                        //ADD SIGNAL HANDLING
                    } else {
                        //usleep(1000);
                        i++; //Passa al prossimo elemento della lista
                        if(i == count) { //Quando li ha mandati tutti
                            _write = TRUE; //Finisce di scrivere
                            freePathList(lista); //Dealloca tutta la lista
                        }
                    } 
                }

                //Read
                if(!_read) {
                    if(read(fd_2[READ], av, 2) > 1) {
                        perc++;
                        if(perc == count) {
                            _read = TRUE;
                        }
                        //system("clear");
                        //printf("Numero file: %d,n=%d m=%d\n[",count,n,m);
                        //for(j = 0; j < perc*10/count; j++) {
                        //    printf("#");
                        //}
                        //for(j += 1; j <= 10; j++) {
                        //    printf(".");
                        //}
                        //printf("]\n");
                    }
                }
                
            }
            
            close(fd_1[READ]);
            close(fd_1[WRITE]);
            close(fd_2[READ]);
            close(fd_2[WRITE]);
            wait(NULL);
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