#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    //Parsing arguments------------------------------------------------------------------------------------------
    int n = 3;
    int m = 4;
    
    //ATTENZIONE: args puo' essere sostituita da filePath qualora questa non sia piu' utile dopo il fork
    //Rimuovere questi commenti alla fine del progetto :)
    node msg; //list used to pass path's to child
    
    //parser variables
    int i; //Variabile usata per ciclare gli argomenti (argv[i])
    int value_return = 0; //Valore di ritorno
    int count = 0; //numero di file univoci da analizzare
    node filePath = NULL; //list of path's strings
    char resolved_path[PATH_MAX];   //contiene il percorso assoluto di un file
    char *tmp;
    char flag = FALSE; // se flag = true, non bisogna analizzare l'argomento. (l'argomento successivo è il numero o di n o di m)
    char setn = FALSE; // se setn = true, n è stato cambiato
    char setm = FALSE; // se setn = true, m è stato cambiato
    char done = FALSE; // true quando l'argomento è stato analizzato
    int dim = 0; //dimensione comando
    char errdir = FALSE;
    FILE *fp;
    char riga[1035];

    //Variables for IPC
    int fd_1[2]; //Pipe
    int fd_2[2];
    pid_t f; //fork return value
    char array[8][20]; //Matrice di appoggio
    char* args[8]; //String og arguments to pass to child
    //system("clear");
    if(argc < 1) { //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_A();
    }
    else {
        for(i = 1; i < argc && value_return == 0; i++) {
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


            }else if(strncmp(argv[i], "-", 1) == 0){//-----ERRORI input strani che iniziano con -
                value_return = err_args_A();
            }

            if (flag == FALSE && value_return == 0){ //------Vuol dire che argv[i] e' un file o una cartella
                /*  Viene utilizzato il comando:  test -d [dir] && find [dir] -type f -follow -print || echo "-[ERROR]"
                    Il comando test -d controlla l'esistenza del file/directory input. In caso di successo viene lanciato find
                    In caso di successo viene lanciato find che restituisce la lista di tutti i file nella cartella e nelle sottocartelle
                    Se l'input non esiste restituisce -[ERROR], in modo che possa essere intercettato dal parser
                */
                char command[strlen("test -d  && find ") + strlen(argv[i])*2 + strlen(" -type f -follow -print || echo \"-[ERROR]\"")+ 1]; //Creazione comando
                strcpy(command, "test -d ");
                strcat(command, argv[i]);
                strcat(command, " && find ");
                strcat(command, argv[i]);
                strcat(command, " -type f -follow -print || echo \"-[ERROR]\"");
                fp = popen(command, "r"); //avvia il comando e in fp prende l'output
                if (fp == NULL) //Se il comando non va a buon fine
                {
                    value_return = err_args_A();
                } else { //Il comando va a buon fine
                    while (fgets(riga, sizeof(riga), fp) != NULL && errdir == FALSE) //Legge riga per riga e aggiunge alla lista
                {
                    if (strcmp(riga,"-[ERROR]\n")){
                        realpath(riga, resolved_path);  //risalgo al percorso assoluto
                        resolved_path[strlen(resolved_path)-1] = 0; //tolgo l'ultimo carattere che manderebbe a capo      
                        tmp = &resolved_path[0];                           
                        if (!(is_present(tmp, filePath))){ //Controlla se il percorso è già presente nella lista
                            filePath = insert_first(tmp,filePath); //aggiunge il percorso alla lista
                            count++; //incrementa il numero di percorsi inseriti con successo
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
        printf("Numero file: %d,n=%d m=%d\n",count,n,m);
    }
    
    //IPC
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_1) == -1) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di git errore setta il valore di ritorno a ERR_PIPE
        }
    }
    if(value_return == 0) { //Testo che non si siano verificati errori in precedenza
        if(pipe(fd_2) == -1) { //Controllo se nella creazione della pipe ci sono errori
            value_return = err_pipe(); //in caso di errore setta il valore di ritorno a ERR_PIPE
        }
    }

    if(value_return == 0) { //same as before
        printf("FORK\n");
        f = fork(); //Fork dei processi
        if(f == -1) { //Controllo che non ci siano stati errori durante il fork
            value_return = err_fork(); //in caso di errore setta il valore di ritorno a ERR_FORK
        }
    }

    if(value_return == 0) { //same
        msg = filePath; //copia il riferimento alla lista cosi' da poterla scorrere senza perdere i riferimanti effettivi
        if(f > 0) { //PARENT SIDE
            printf("START parent: %d\n", getpid());
            while(msg != NULL && value_return == 0) { //cicla su tutti gli elementi della lista
                if (write(fd_1[WRITE], msg->path, PATH_MAX) == -1) {
                    value_return = err_write();
                    //ADD SIGNAL HANDLING
                }
                msg = msg->next;
            }
            close(fd_1[WRITE]);
        }
        wait(NULL);
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE
            printf("START son: %d\n", getpid());

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

            dup2(fd_2[WRITE], STDIN_FILENO); //close STDOUT_FILENO and open fd[WRITE]
            dup2(fd_1[READ], STDIN_FILENO);
            execvp(args[0], args);
        }
    } 
    
    return value_return;
}