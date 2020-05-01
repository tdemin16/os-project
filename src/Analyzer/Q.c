//version 1.4 - Updated to 2020.05.01
#include "../lib/lib.h"

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int main(int argc, char *argv[])
{
    initialize_vector();
    char *DIR = argv[1];       // setup the directory
    int begin = atoi(argv[2]); //setup start of the process in the file
    int end = atoi(argv[3]);   //setup end of the process in the file

    FILE *fp = fopen(DIR, "r"); //open in read mode the file in the directory
    if (fp == NULL)
    {
        printf("[!] Errore nell'apertura del file'");
        //exit(1); //serve(?)
    }

    get_subset(fp, begin, end); //getting all the chars

    print_vector(); //U

    fclose(fp);
    printf("\n");
    return 1;
}


