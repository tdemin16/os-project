#include "./lib/lib.h"

//ret_val -> value_return (standard progetto)
//usa pid_t al posto di int per i pid (stessa roba ma sarebbe da usare quello)
//Dichiara una variabile char* args[]
//args[0] = "./A";
//args[i] argomenti
//execv -> execvp(args[0], args)
//aggiungi il controllo sulla value_return ogni volta che un blocco viene eseguito subito dopo un possibile errore
//G non serve. Non e` da progetto
//pidA = wait(&stateA); aspetta all'infinito perche` G e` in un ciclo infinito
//valuta possibili errori sulla call "kill"

int main(int argc, char *argv[]) {
	//------------------------------ DEFINIZIONE VARIABILI
	int ret_val = 0;
	int n,m;
	int pidA, stateA, pidG;		//G è il processo che si occupa di stampare i punti "."
	int running_A = TRUE;		//[da rimuovere] serve solo per la prova dei punti "."
	
	if(argc==1) {			//[da rimuovere] (solo un promemoria di come eseguire, per me - Benedetta)
		printf("Try command: ./M ./../src/Analyzer\n");	//[da rimuovere]
		return -1; 		//[da rimuovere]
	} 
	
	//------------------------------ LANCIO DI A COME FIGLIO
	pidA = fork();
	if(pidA == -1) {
		//[errore] can't fork
	}
	if(pidA == 0) { 		//mi trovo nel figlio
		sleep(3);		//come prova del funzionamento dei punti di caricamento "."
		printf("\n\n");
		execv("./A",argv); 	//lancio effettivo di A in un processo che avrà pid=pidA
	}
	
	//--------------------------------------------------------------- DA QUI IN POI SOLO PADRE
	
	//------------------------------ PROCESSO G CHE STAMPA I PUNTINI "."
	pidG = fork();
	if (pidG == -1) {
		//[errore] can't fork
	} 
	if (pidG == 0) {
		while(running_A) {	//[da rimuovere] giusto per non mettere while(1) per ora 
			printf("."); fflush(stdout);
			sleep(1);	//in modo che stampi un "." ogni secondo, cambiabile
		}
	} else {

	//------------------------------ SOLO PADRE - Attesa A e chiusura G
		pidA = wait(&stateA);	//[da rimuovere] so che non va usato, è solo di prova per "."
		running_A = FALSE;
		kill(pidG,SIGKILL);	//chiude il processo G di grafica pallini
		printf("\nFine esecuzione A. Value_return = %d\n",WEXITSTATUS(stateA));
	}	
	return ret_val;
}
