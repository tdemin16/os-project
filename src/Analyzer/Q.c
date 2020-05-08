//Updated to 2020.05.02
#include "../lib/lib.h"

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int main(int argc, char *argv[])
{   
    //Arguments passed
    int part;
    int m;
    int v[DIM_V];
    int value_return = 0;
    FILE* fp;
    char* freq;

    //IPC Arguments
    char path[PATH_MAX];
    int _write = FALSE;

    //char resp[DIM_RESP] = "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94\0";
    char resp[DIM_RESP];

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
                //printf("Q: %s Arrivato\n", path);
                if(strcmp(path, "///") == 0){
                    _write = TRUE;
                    if(write(STDOUT_FILENO, path, PATH_MAX) == -1) {
                        value_return = err_write();
                    }
                } else {
                    fp = fopen(path, "r");
                    if(fp == NULL) {
                        value_return = err_file_open();
                    } else {
                        //getfrequencies;
                        createCsv(v,resp);
                        if(write(STDOUT_FILENO, resp, DIM_RESP) == -1) {
                            value_return = err_write();
                        }
                    }
                }
            }
        }
    }

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