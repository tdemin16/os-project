//Version updated to 2020.05.02
//P si occupano di gestire ciascuno un sottoinsieme degli input (partizionato in “n” sottogruppi) creando a sua volta “m” figli
#include "../lib/lib.h"

int main(int argc, char *argv[])
{
    char *DIR = "./text_file/prova1.txt"; // setup the directory
    int value_return;
    int i;
    char *command = malloc(sizeof(char));
    char *begin = malloc(sizeof(char));
    char *end = malloc(sizeof(char));
    int m = atoi(argv[1]); //setup m
    m_process *div = malloc(sizeof(m));

    if (argc - 1 != 1)
    {
        value_return = err_args_P();
    }
    else
    {
        FILE *fp = fopen(DIR, "r"); //open the file

        if (fp == NULL)
        {
            value_return = err_file_open(); //error if file is in
        }
        else
        {
            div = splitter(fp, m);

            for (i = 0; i < m; i++) //Generate 4 process
            {
                itoa(div[i].begin, begin, 10);
                itoa(div[i].end, end, 10);
                strcat(command, "./Q "); //From here
                strcat(command, begin);  //Creates the
                strcat(command, " ");    //command for system
                strcat(command, end);    //To here
                printf("\n%s\n\n", command);
                //system(command);
                memset(command, 0, sizeof(*command)); //Empty command string
            }
        }
        fclose(fp);
    }

    free(command);
    free(begin);
    free(end);
    free(div);

    return value_return;
}
