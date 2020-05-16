#include "./lib/lib.h"
#include <unistd.h>

#define DIM_CMD 1024*4
 
int check_command(char *str);	//controlla il comando un numero corrispondente: -1 comando non riconosciuto ecc
//int report(); 		//potrebbe essere la funzione che si occupa di attivare/disattivare R


int main(int argc, char *argv[]) {
	//------------------------------ DEFINIZIONE VARIABILI
	int ret_val = 0;
	int n,m;
	int pidA, stateA;	//al momento non necessarie
	char cmd[DIM_CMD];	//contiente il comando lanciato
	char ch = '0';		//per leggere un carattere per volta
	int end = FALSE;	//TRUE se lanciato il comando "close" 
	
	//------------------------------ LANCIO DI A COME FIGLIO
	pidA = fork();
	if(pidA == -1) {
		//[errore] can't fork
	}
	if(pidA == 0) { //------------- FIGLIO A
		printf("Avviato A.\n"); //[da rimuovere]
		execv("./R",argv); 	//[da modificare] qui verrà avviato A
	} else { //--------------------- PADRE M
		while(!end) {//--------- CICLO DI ATTESA COMANDI IN INPUT
                       	strcpy(cmd,"");  //svuota la stringa per il prossimo comando
                       	printf("\n"); printf("Analyzer>"); fflush(stdout); 
                	ch = '\0';
                       	while(ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
				ch = getc(stdin);
				if(ch != '\n') strcat(cmd,&ch);	//concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
			}
			int res_cmd = check_command(cmd);
			if (res_cmd == -1) {
				//richiama la funzione help() coi comandi
				printf("Comando inserito non corretto\n");
			}
			printf("\n");
                	if (res_cmd==0) end = TRUE;  //con il comando close interrompe il ciclo
                } 
		pidA = wait(&stateA);
                printf("HO FINITO\n");
	}
	
	return ret_val;
}

/*comandi possibili:
setn int setm int 	[da gestire]
info			
info -c //clustered info
close
help			[da modificare
*/
int check_command(char *cmd) {			
	int res=-1;	//errore input comando
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
                } 
	} 
	return res;
}
