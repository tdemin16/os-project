//Version updated to 2020.05.02
//P si occupano di gestire ciascuno un sottoinsieme degli input (partizionato in “n” sottogruppi) creando a sua volta “m” figli
#include "../lib/lib.h"

int main(int argc, char *argv[])
{

    int value_return;

    if (argc - 1 != 1)
    {
        value_return = err_args_P();
    }
    else
    {
        char *DIR = "./prova.txt"; // setup the directory
        int m = atoi(argv[1]);     //setup m

        FILE *fp = fopen(DIR, "r");

        if (fp == NULL)
        {
            value_return = err_file_open();
        }
        else
        {
            int file_length = file_len(fp);   //get the length of the file in terms of chars
            int division = (file_length / m); //get the subdivision of the files //DA IMPLEMENTARE CONTROLLO QUANDO SI HA UN NUMERO CHE NON E' INTERO IN DIVISION --> IL FILE VA DIVISO NON EQUAMENTE
            int i;
            printf("%d e %d\n", file_length, division);
            for (i = 0; i < m; i++) //Generate 4 process
            {
                int inizio = i * division;
                int fine = ((i + 1) * division);
                printf("Da dove inizia il file: %d (non compreso), e finisce a: %d\n", inizio, fine);
            }
        }
        fclose(fp);
    }

    return value_return;
}
