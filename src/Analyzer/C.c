#include "../lib/lib.h"

int main(int argc, char const *argv[]) {
    
    //Interrupt initialize
    signal(SIGINT,handle_sigint);
    processes* proc;

    int value_return = 0;
    int nfiles = 0; //number of files to retreive from pipe
    int n = 3;
    int m = 4;
    int i;
    int j;
    int k;
    char path[PATH_MAX];
    char resp[DIM_RESP];
    char sum[DIM_RESP];
    int v[DIM_V];
    int part_received = 0;
    int count = 0; //Maintain the current amount of files sended
    
    //IPC Variables
    int* fd;
    pid_t f = getpid();
    int id; //Indica il numero del figlio (necessario per calcolare quale pipe utilizzare)
    int size_pipe; //Size of pipes
    char array[2][4];
    char* args[3];
    int _read = FALSE; //Indica se ha finito di leggere dai figli
    int _write = FALSE; //Indica se ha finito di scrivere
    char ad[2];
    strcpy(ad, "$");
    
    //Parsing arguments------------------------------------------------------------------------------------------
    if(argc % 2 == 0 || argc < 2) { //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_C();
    }
    else {
        for(i = 1; i < argc && value_return == 0; i += 2) {
            if(!strcmp(argv[i], "-nfiles")) { //Check if the argument is equal to -nfiles
                nfiles = atoi(argv[i + 1]);
                if(nfiles == 0) value_return = err_args_C();
            } 
            else if(!strcmp(argv[i], "-setn")) {//Check if argument is equal to -setn
                n = atoi(argv[i + 1]);
                if(n == 0) value_return = err_args_C();
            }
            else if(!strcmp(argv[i], "-setm")) { //Check if argument is equal to -setm
                m = atoi(argv[i+1]);
                if(m == 0) value_return = err_args_C();
            }
            else {
                value_return = err_args_C();
            }
        }
        if(nfiles == 0 && value_return == 0) value_return = err_args_C(); //Check if nfiles is setted, if not gives an error (value_return used to avoid double messages)
    }

    initialize_vector(v);

    //Generating pipes-------------------------------------------------------
    if(value_return == 0) {
        //Crea n*4 pipes (4 per coppia padre figlio, 2 in lettura e 2 in scrittura)
        size_pipe = n*4;
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
        if(unlock_pipes(fd, size_pipe) == -1) { //Set nonblocking pipes
            value_return = err_fcntl();
        }
        if(fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
            value_return = err_fcntl();
        }
    }

    //Allocation of proc and initialization of all n chars* is_open
    proc = malloc(n);
    initialize_processes(proc,n);

    //Forking----------------------------------------------------------------
    if(value_return == 0) {
        //Ciclo n volte, controllando che f > 0 (padre) e non ci siano errori -> genera quindi n processi
        for(i = 0; i < n && f > 0 && value_return == 0; i++) {
            f = fork();
            if(f == 0) { //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
                id = i;
                insert_process(f,proc); //insert process in list of OPEN processes
            }
            if(f == -1) { //Controllo che non ci siano stati errori durante il fork
                value_return = err_fork(); //In caso di errore setta il valore di ritorno a ERR_FORK
            }
        }
    }

    //----------------------------------------------------------------------------------------
    
    if(value_return == 0) {
        i = 0;
        k = 0;
        if(f > 0) { //PARENT SIDE
            while(value_return == 0 && (!_read || !_write)) {

                //Write
                if(!_write) {
                    if(read(STDIN_FILENO, path, PATH_MAX) > 0) { //Prova a leggere dalla pipe
                        //printf("C: %s arrivato\n",path);
                        if(write(fd[i*4 + 3], path, PATH_MAX) == -1) { //Test write
                            value_return = err_write();
                            //ADD SIGNAL HANDLING
                        } else {
                            count++;
                            i = (i+1) % n;
                        }
                    }
                    if(count == nfiles) { //Ha passato tutti i path ai figli
                        _write = TRUE;
                        strcpy(path, "///");
                        for(j = 0; j < n; j++) { //Manda a tutti i processi P la fine della scrittura
                            if(write(fd[j*4 + 3], path, PATH_MAX) == -1) {
                                value_return = err_write(); //VA IN ERRORE QUA, SE COMMENTATE NON DA PIU' ERRORE, Q RICEVE COMUNQUE LE STRINGHE
                            }
                        }
                    }
                }

                //Read
                if(!_read) {
                    if(read(fd[k*4 + 0], resp, DIM_RESP) > 0) {
                        
                        if(strcmp(resp, "///") == 0) {//Lascia questo blocco
                            part_received++;
                            if(part_received == n) {
                                _read = TRUE;
                            }
                        } else { 
                            //printf("[+] - %s\n",strtok(resp, "#"));
                            addCsvToArray(resp,v);
                            if(write(STDOUT_FILENO, ad, 2) == -1) {
                                value_return = err_write();
                            }
                        }
                    }
                    k = (k+1) % n;
                }
            }
            close_pipes(fd, size_pipe);
            free(fd);
            //createCsv(v,sum);
            //printStat_Cluster(sum);
        }
    }

    if(value_return == 0) {
        if(f == 0) { //SON SIDE
            //Creates char args
            strcpy(array[0], "./P");
            sprintf(array[1], "%d", m);
            args[0] = array[0];
            args[1] = array[1];
            args[2] = NULL;

            dup2(fd[id*4 + 2], STDIN_FILENO);
            dup2(fd[id*4 + 1], STDOUT_FILENO);
            close_pipes(fd, size_pipe);
            free(fd);
            
            if(execvp(args[0], args) == -1) { //Test exec
                value_return = err_exec(errno); //Set value return
            }
        }
    }

    for (i = 0; i < n; i++) //deallocate the n-proc[]->is_open
    {
        free(proc[i].is_open);
    }

    free(proc); //deallocate proc

    return value_return;
}

//NON NECESSARIO, MANTENUTO PER SICUREZZA---------------------------------------------------
/*if(value_return == 0) {
    if(f > 0) { //PARENT SIDE
        fileLeft = nfiles;
        processLeft = n;
        tmpFiles = 0;

        while (processLeft >0){ //Ciclo fino a che non ho assegnato i file a ogni processo
            printf("Process %d: %d files\n",n-processLeft+1,(int)((float)fileLeft/(float)processLeft));

            tmpFiles = (int)((float)fileLeft/(float)processLeft); //Divido i file rimanenti per i processi rimanenti e tengo il numero troncato
            for (i = 0; i<tmpFiles; i++){ //Leggo il numero di file assegnato al processo
                if(read(STDIN_FILENO, path, PATH_MAX) == 0) {

                }
            }

            fileLeft-=tmpFiles;
            processLeft-=1;
        }
    }
}*/