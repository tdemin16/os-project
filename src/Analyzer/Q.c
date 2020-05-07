//Updated to 2020.05.02
#include "../lib/lib.h"

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int main(int argc, char *argv[])
{   
    //Arguments passed
    int part;
    int m;

    int value_return = 0;
    FILE* fp;

    //IPC Arguments
    char* path;
    int _write = FALSE;


    //Parsing Arguments--------------------------------------------------------------------
    if(argc != 3) {
        value_return = err_args_Q();
    } else {
        m = atoi(argv[2]);
        if(m == 0) value_return = err_m_not_valid();
    }
    if(value_return == 0) {
        part = atoi(argv[1]);
        if(part >= m) value_return = err_part_not_valid();
    }

    if(value_return == 0) {
        while(value_return == 0 && !_write) {
            if(read(STDIN_FILENO, path, PATH_MAX) > 0) {
                //printf("%d: %s\n", part, path);
                if(strcmp(path, "///") == 0) {
                    _write = TRUE;
                } else {
                    fp = fopen(path, "r");
                    if(fp == NULL) {
                        value_return = err_file_open();
                    } else {
                        //getfrequencies;
                    }
                    if(value_return == 0) {
                        //write
                    }
                }
            }
        }
    }

    printf("Q chiuso\n");
    return value_return;
}

/* ANDREA L'HO RISCRITTO PER POTER INSERIRE LA COMUNICAZIONE. HO PERO' USATO QUELLO CHE HAI
   SCRITTO TU PER L'ELABORAZIONE------------------------------------------------------------

   if (argc != 3) {
        value_return = err_args_Q();
    }
    else
    {
        char *DIR = "./README.md"; // setup the directory
        int part = atoi(argv[1]);            //setup start of the process in the file
        int m = atoi(argv[2]);              //setup end of the process in the file

        if (m < part)
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
                //get_subset(fp, v, begin, end); //getting all the chars
                get_frequencies(fp, part, m);
                //print_vector(v); //U
            }
            fclose(fp);
        }
    }
*/