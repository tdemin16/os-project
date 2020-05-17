#include "./lib/lib.h"
 
int check_command(char *str);	//controlla il comando un numero corrispondente: -1 comando non riconosciuto ecc
//int report(); 		//potrebbe essere la funzione che si occupa di attivare/disattivare R

process *p; //Declaring p (it's global because hendle_sigint can't have parameters, only int sig)

void handle_sigint(int sig){
    printf("\n[!] Ricevuta terminazione, inizio terminazione processi ... \n");
    int i = p->count-1; //start from the end
    while (i != 0) //while we haven't controlled every single process
    {
        if (p->pid[i] > 0) //Processo padre
        {
            if (kill(p->pid[i],9) == 0){ //Tries to kill process with pid saved in pid[i]
                printf("\tProcesso %d terminato con successo!\n",p->pid[i]); //if it success you terminated it correctly
            }else{
                printf("\t[!] Errore, non sono riuscito a chiudere il processo %d!",p->pid[i]); //if it fail something is wrong
            }  
        }/*else if(proc[i] == 0){
            if (kill(proc[i],9))
            {
                printf("Ucciso processo figlio");
            }  
        }*/
        i--; //i-- otherwise it will go to infinity      
    }
    freeList(p); //free memory allocated for p 
    printf("[!] ... Chiusura processi terminata\n");
    exit(-1); //return exit with error -1
}

int main(int argc, char *argv[]) {

	signal(SIGINT,handle_sigint); //Handler for SIGINT (Ctrl-C)

	//------------------------------ DEFINIZIONE VARIABILI
	p = create_process(1); //Allocate dynamically p with dimension 1
	int value_return = 0;
	int n,m;
	pid_t f;			//al momento non necessarie
	char cmd[DIM_CMD];	//contiente il comando lanciato
	char ch = '0';		//per leggere un carattere per volta
	int end = FALSE;	//TRUE se lanciato il comando "close"
	int res_cmd;
	int _write = TRUE;

	//IPC Variables--------------------------------------------------
	int fd[2];
	char array_args[4];

	if(argc == 1) {
		value_return = err_args_M();
	}

	//IPC
	if(value_return == 0) {
		if(pipe(fd) != 0) {
			value_return = err_pipe();
		}
	}

	//Set Non-blocking pipes (Shouldn't block anyway, just to be sure)
	if(value_return == 0) {
		if(fcntl(fd[WRITE], F_SETFL, O_NONBLOCK)) {
			value_return = err_fcntl();
		}
		if(fcntl(fd[READ], F_SETFL, O_NONBLOCK)) {
			value_return = err_fcntl();
		}
	}
	
	if(value_return == 0) {
		f = fork();
		if(f == -1) {
			value_return = err_fork();
		}
	}

	//--------------------------------------------------------------------------------
	if(value_return == 0) {
		insertProcess(p,getpid());
		if(f > 0) { //PARENT SIDE
			insertProcess(p,f);
			while(!end && value_return == 0) {//--------- CICLO DI ATTESA COMANDI IN INPUT
            	strcpy(cmd,"");  //svuota la stringa per il prossimo comando
            	printf("\n"); printf("Analyzer> "); fflush(stdout); 
            	ch = '\0';
            	while(ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
					ch = getc(stdin);
					if(ch != '\n') strcat(cmd,&ch);	//concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
				}

				res_cmd = check_command(cmd);
				if (res_cmd == -1) {
					//richiama la funzione help() coi comandi
					printf("Comando inserito non corretto\n");
				}
				printf("\n");
            	if(res_cmd == 0) end = TRUE;  //con il comando close interrompe il ciclo
				if(res_cmd == 2) {
					while(value_return == 0 && _write) {
						if(write(fd[WRITE], cmd, DIM_CMD) == -1) {
							if(errno != EAGAIN) {
								value_return = err_write();
							}
						} else {
							_write = FALSE;
						}
					}
					_write = TRUE;
				}
        	}
			close(fd[WRITE]);
			close(fd[READ]);
        	printf("HO FINITO\n");
		}
	}

	if(value_return == 0) {
		if(f == 0) { //A SIDE
			
			//Change binary file
			strcpy(array_args, "./A");
			argv[0] = array_args;

			//Redirects pipes
			dup2(fd[READ], STDIN_FILENO);

			//Close pipes
			close(fd[WRITE]);
			close(fd[READ]);
			
			//Change code with A
			//if(execvp(argv[0], argv) == -1) {
			//	value_return = err_exec(errno);
			//}
		}
	}
	
	return value_return;
}

/*comandi possibili:
setn int setm int 	[da gestire]
info			
info -c //clustered info
close
help			[da modificare
*/

//Codici Univoci
//COdice per identificare un comando che deve essere mandato ad A. Per il momento io uso '2'
int check_command(char *cmd) {			
	int res = -1;	//errore input comando
	if(strlen(cmd)>3) { 	//possibile che sia un comando accettabile
		if(strstr(cmd,"help")!=NULL) {
			//mostra help comandi
			printf("Ci sarà una funzione help comandi\n");
			res = 1;
		} else if (strstr(cmd,"close")!=NULL) {
			res = 0;
		} else if (strstr(cmd,"info")!=NULL) { 
            //apre processo R
            printf("\nAperta comunicazione con R\n"); //[da eliminare]
            res = 1;
		} else {//Provvisorio per testare A in quanto manca il branch per i comandi da mandargli
			res = 2;
		}
	} 
	return res;
}
