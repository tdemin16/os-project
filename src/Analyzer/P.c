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

    //IPC Variables
    int* fd;
    int size_pipe;
    int f = getpid();
    int id; //Identifica il numero del figlio generato
    int _read = FALSE;
    int _write = FALSE;
    char array[4][4];
    char* args[4];


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
    }


    //Forking-----------------------------------------------------------
    if(value_return == 0) {
        //Ciclo m volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi m processi
        for(i = 0; i < m && f > 0 && value_return == 0; i++) {
            f = fork();
            if(f == 0) { //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
                id = i;
            }
            if(f == -1) {
                value_return = err_fork();
            }
        }
    }


    //----------------------------------------------------------------------

    if(value_return == 0) {
        if(f > 0) { //PARENT SIDE
            while(value_return == 0 && (/*1_read ||*/ !_write)) {

                //Write
                if(!_write) {
                    if(read(STDIN_FILENO, path, PATH_MAX) > 0) { //Prova a leggere dalla pipe
                        if(strcmp(path, "///") == 0) {
                            _write = TRUE;
                            for(i = 0; i < m; i++) { //Manda a tutti i processi Q la fine della scrittura
                                if(write(fd[i*4 + 3], "///", PATH_MAX) == -1) {
                                    value_return = err_write();
                                }
                            }
                        } 
                        else {
                            for(i = 0; i < m; i++) { //Cicla su tutti i processi m
                                if(write(fd[i*4 + 3], path, PATH_MAX) == -1) { //Test Write
                                    value_return = err_write();
                                }
                            }
                        }
                    }
                }

                //Read
                /*if(!_read) {

                }*/
            }
            wait(NULL);
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
            //dup2(fd[id*4 + 1], STDOUT_FILENO);
            close_pipes(fd, size_pipe);

            if(execvp(args[0], args) == -1) {
                value_return = err_exec(errno);
            }
        }
    }

    return value_return;
}
