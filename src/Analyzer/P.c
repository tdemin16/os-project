//Version updated to 2020.05.02
//P si occupano di gestire ciascuno un sottoinsieme degli input (partizionato in “n” sottogruppi) creando a sua volta “m” figli
#include "../lib/lib.h"

int main(int argc, char *argv[])
{

    int value_return;
    int i;

    if (argc - 1 != 1)
    {
        value_return = err_args_P();
    }
    else
    {
        char *DIR = "./prova.txt"; // setup the directory
        int m = atoi(argv[1]);     //setup m
        m_process* div;

        FILE *fp = fopen(DIR, "r"); //open the file

        if (fp == NULL)
        {
            value_return = err_file_open(); //error if file is in
        }
        else
        {
            div = splitter(fp,m);

            //printf("%d e %d e %f e %d\n", file_length, int_div, double_div, rest); //U
            for (i = 0; i < m; i++)                                                //Generate 4 process
            {
                printf("Da dove inizia il file: %d (non compreso), e finisce a: %d\n", div[i].begin, div[i].end); //U
            }
        }
        fclose(fp);
    }

    return value_return;
}
