//Version updated to 2020.05.02
//P si occupano di gestire ciascuno un sottoinsieme degli input (partizionato in “n” sottogruppi) creando a sua volta “m” figli
#include "../lib/lib.h"

int main(int argc, char *argv[])
{  
    //Argument passed
    int m = 4;

    int value_return = 0;
    int i;
    char path[PATH_MAX];
    char failedPath[m][PATH_MAX];
    char resp[DIM_RESP];

    //IPC Variables
    int* fd;
    int size_pipe;
    int f = getpid();
    int id; //Identifica il numero del figlio generato
    int _read = FALSE;
    int _write = FALSE;
    char array[4][4];
    char* args[4];
    int count = 0;


    //Parsing arguments-------------------------------------------------------
    if(argc != 2) {
        value_return = err_args_P();
    } else {
        m = atoi(argv[1]);
        if(m == 0) value_return = err_m_not_valid();
    }


    //Generating pipes-------------------------------------------------------
    if(value_return == 0) {
        //Crea m*4 pipes (4 per coppia padre figlio, 2 in lettura e 2 in scrittura)
        size_pipe = m*4;
        fd = (int*) malloc(size_pipe * sizeof(int));
        //Alloco le pipes a due a due
        for(i = 0; i < size_pipe-1; i += 2) {
            if(pipe(fd + i) == -1) { //Controlla se ci sono errori nella creazione della pipe
                value_return = err_pipe(); //In caso di errore setta il valore di ritorno
            }
        }
    }
    /*  PIPES ENCODING---------------------------------------------
        fd[id*4 + 0] PARENT READ
        fd[id*4 + 1] SON WRITE
        fd[id*4 + 2] SON READ
        fd[id*4 + 3] PARENT WRITE
    *///-----------------------------------------------------------

    if(value_return == 0) {
        if(unlock_pipes(fd, size_pipe) == -1) { //Set unblocking pipes
            value_return = err_fcntl();
        }
        if(fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }

    //Forking-----------------------------------------------------------
    if(value_return == 0) {
        //Ciclo m volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi m processi
        for(i = 0; i < m && f > 0 && value_return == 0; i++) {
            f = fork();
            if(f == 0) { //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
                id = i;
            }
            else if(f == -1) {
                value_return = err_fork();
            }/*else{
                //insert_process(f);
            }*/
        }
    }

    char stop = FALSE;
    for (i=0; i<m; i++){
        failedPath[i][0]='\0';
    }
    int cc = 0;
    char sent = TRUE;
    char end= FALSE;
    int terminated[m];
    int test=0;
    for (i= 0; i<m; i++){
        terminated[i] = FALSE;
    }
    //----------------------------------------------------------------------
    if(value_return == 0) {
        if(f > 0) { //PARENT SIDE
            while(value_return == 0 && (!_read || !_write)) {
                //Write
                if(!_write) { //Se non ha finito di scrivere
                    if (sent){// se il file è stato mandato a tutti i q, leggo il prossimo 
                        if(read(STDIN_FILENO, path, PATH_MAX) > 0) { //provo a leggere
                            //QUI BISOGNA AGGIUNGERE IL SUM DEI Q DIVISI PER FILE
                            if ((!strncmp(path,"///",3))&& sent == TRUE){ //Se leggo una stringa di terminazione
                                end = TRUE; //Setto end a true
                            }
                            for(i = 0; i < m; i++) { //Provo a inviare path a tutti i Q
                                if(write(fd[i*4 + 3], path, PATH_MAX) == -1) {
                                        if (errno != EAGAIN){
                                            value_return = err_write();
                                        } else {
                                            sent = FALSE; //Se non ci riesce setta sent a false
                                            terminated[i] = FALSE;
                                        }
                                } else {
                                    terminated[i] = TRUE;
                                }
                            }
                        }
                    } else {
                        //fprintf(stderr,"C[%d] provo ritrasmissione di %s\n",getpid(),path);
                        sent = TRUE;
                        for(i = 0; i < m; i++) {
                            if(!terminated[i]){ //Per ogni path non inviato, riprova
                                if(write(fd[i*4 + 3], path, PATH_MAX) == -1) {
                                    if (errno != EAGAIN){
                                        value_return = err_write();
                                    } else {
                                        sent = FALSE; //sent rimane TRUE se durante l'invio non ci sono stati problemi
                                    }
                                } else {
                                    terminated[i] = TRUE;
                                }
                            }
                        }
                        
                    }
                    if (end && sent == TRUE){ //Se lo stato e' end, e tutto e' stato inviato, allora la write e' finita
                            //fprintf(stderr,"C finito di scrivere, %s\n",path);
                        _write = TRUE;
                    }
                }
                
                //Read
                
                if(!_read) {
                    for(i = 0; i < m; i++) { //Cicla tra tutti i figli
                        if(read(fd[i*4 + 0], resp, DIM_RESP) > 0) { //Legge la pipe del figlio i
                            if(strcmp(resp, "///") == 0) { //Controlla se e` la fine del messaggio
                                count++; //Conta quanti terminatori sono arrivati
                                if(count == m) { //Quando tutti i figli hanno terminato
                                    _read = TRUE; //Ha finito di leggere
                                    if(write(STDOUT_FILENO, resp, DIM_RESP) == -1) { //Scrive il carattere di teminazione
                                        if (errno != EAGAIN){
                                            value_return = err_write();
                                        } else {
                                            fprintf(stderr,"P->C: Pipe piena\n");
                                        }
                                    }
                                }
                            } else { //Se non e` la fine del messaggio
                                if(write(STDOUT_FILENO, resp, DIM_RESP) == -1) { //Invia la stringa resp
                                    if (errno != EAGAIN){
                                        value_return = err_write();
                                    } else {
                                        fprintf(stderr,"P->C: Pipe piena\n");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            close_pipes(fd, size_pipe);
            free(fd);
        }
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE
            //Creates char args
            strcpy(array[0], "./Q");
            sprintf(array[1], "%d", id);
            sprintf(array[2], "%d", m);
            args[0] = array[0];
            args[1] = array[1];
            args[2] = array[2];
            args[3] = NULL;

            dup2(fd[id*4 + 2], STDIN_FILENO);
            dup2(fd[id*4 + 1], STDOUT_FILENO);
            close_pipes(fd, size_pipe);
            free(fd);

            if(execvp(args[0], args) == -1) {
                value_return = err_exec(errno);
            }
        }
    }
    
    /*for (i = 0; i < m; i++) //deallocate the m-proc[]->is_open
    {
        free(proc[i].is_open);
    }
    
    free(proc); //deallocate proc*/

    return value_return;
}
