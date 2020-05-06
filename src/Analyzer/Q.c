//Updated to 2020.05.02
#include "../lib/lib.h"

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int main(int argc, char *argv[])
{
    int value_return;
    int v[DIM_V];
    initialize_vector(v);

    if (argc - 1 != 2)
    {
        value_return = err_args_Q();
    }
    else
    {
        char *DIR = "./text_file/prova1.txt"; // setup the directory
        int begin = atoi(argv[1]);            //setup start of the process in the file
        int end = atoi(argv[2]);              //setup end of the process in the file

        if (end < begin)
        {
            value_return = err_m_not_valid();
        }
        else
        {
            FILE *fp = fopen(DIR, "r"); //open in read mode the file in the directory

            if (fp == NULL)
            {
                value_return = err_file_open();
            }
            else
            {
                get_subset(fp, v, begin, end); //getting all the chars

                print_vector(v); //U
            }
            fclose(fp);
        }
    }

    printf("\n");
    return value_return;

}
