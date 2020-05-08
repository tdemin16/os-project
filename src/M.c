#include "./lib/lib.h"

#define DIMCOM 40

char com[DIMCOM];   //comando inserito dall'utente

int main(int argc, char const *argv[]) {
	printf("---------------- MAIN ----------------\n"); 
	char tmp;  // variabile temporanea per ottenere il primo carattere
	do {	//------------------------------------------------------------------------------- ciclo eseguito fino al comando "quit"
		printf(" > "); fflush(stdin); scanf("%s",com);	//------------------------------- input da tastiera del comando da eseguire
		tmp = com[0];
		switch(tmp) { 	//--------------------------------------------------------------- switch temporaneo | da rimuovere e sostituire con vari else-if 
			case 'a':
				system("echo \"\033[30;1mViene fatto partire A(?)\033[0m\"");  // comando bash con system per cambiare colore al font, non necessario
				//printf("Viene fatto partire A(?)");  //------------------------ printf sostitutivo senza colore (si può cambiare in C?)
				break;
			case 'r':
				system("echo \"\033[30;1mRisultati stampati con R\033[0m\"");  // vedi sopra ...
				//printf("Risultati stampati con R");
				break;
			case 'h':
				system("echo \"\033[30;1mComandi:\nanalyze - to start the analyzer\nresults - to show here results\nhelp - to view this message\nquit\033[0m\"");
				//printf("Comandi:\nanalyze - to start the analyzer\nresults - to show here results\nhelp - to view this message\nquit");
				break;
			case 'q':
				break;
			default:
				system("echo \"\033[30;1mComando non riconosciuto, type 'help' to view commands\033[0m\"");  // qui andrà l'errore da definire dentro lib
				//printf("Comando non riconosciuto, type 'help' to view commands");
		}
	} while (tmp!='q');
	return 0; 
}
