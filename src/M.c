#include "./lib/lib.h"
#include <unistd.h>

#define DIM_CMD 1024*4
 

int main(int argc, char *argv[]) {
	//------------------------------ DEFINIZIONE VARIABILI
	int ret_val = 0;
	int n,m;
	int pidA, stateA;	//al momento non necessarie
	char cmd[DIM_CMD];	//contiente il comando lanciato
	char ch = '0';		//per leggere un carattere per volta
	int end = FALSE;	//TRUE se lanciato il comando "close"
	
	if(argc==1) {			//[da rimuovere] (solo un promemoria di come eseguire, per me - Benedetta)
		printf("Try command: ./m ./../src/Analyzer\n");	//[da rimuovere]
		return -1; 		//[da rimuovere]
	} 
	
	//------------------------------ LANCIO DI A COME FIGLIO
	pidA = fork();
	if(pidA == -1) {
		//[errore] can't fork
	}
	if(pidA == 0) { //------------- FIGLIO A
		printf("Avviato A.\n");
		execv("./A",argv); 	//lancio effettivo di A in un processo che avrà pid=pidA
	} else { //--------------------- PADRE M
		pidA = wait(&stateA);	//attende le varie stampe di A e le sue cose
		while(!end) {//--------- CICLO DI ATTESA COMANDI IN INPUT
                       	strcpy(cmd,"");  //svuota la stringa per il prossimo comando
                       	printf("Analyzer>"); 
                	ch = '\0';
                       	while(ch != '\n') {  //fino al "lancio" (invio, '\n') del comando continua a leggere caratteri
				ch = getc(stdin);
				if(ch != '\n') strcat(cmd,&ch);	//concatena il carattere alla stringa cmd, ma evita di concatenare '\n'
			}

                	printf("Command: %s\n",cmd);	//stampa per controllare il contenuto di cmd dopo aver "lanciato" il comando
                	if (strcmp(cmd,"close")==0) end = TRUE;  //con il comando close interrompe il ciclo
                } 
                printf("HO FINITO\n");
	}
	
	return ret_val;
}
