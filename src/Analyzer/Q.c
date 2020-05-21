//Updated to 2020.05.02
#include "../lib/lib.h"

//IMPORTANTE DA RICORDARE: una volta ottenuto il char viene convertito in ASCII ed il suo valore diminuito di 32 per evitare di lasciare 32 spazi vuoti nel vettore in cui lo salvo, quindi per stampare nuovamente il dato bisogna aggiungere 32
//Ove presente, 'U' sta ad indicare che quella funzione/parte di codice serve solamente se si vuole verificarne il funzionamento nel processo singolo, NON va utilizzato al di fuori di questo programma, nella release finale li toglieremo

int main(int argc, char* argv[]) {
    //Arguments passed
    int part;
    int m;
    int v[DIM_V];
    int value_return = 0;
    FILE* fp;
    char* id;
    char* analyze;
    int i;
    char* tmpDup = NULL;

    //IPC Arguments
    char path[PATH_MAX];
    int _close = FALSE;
    char respSent = FALSE;
    char resp[DIM_RESP];  // = "1044,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647,2147483647";

    //Parsing Arguments--------------------------------------------------------------------
    if (argc != 3) {
        value_return = err_args_Q();
    } else {
        m = atoi(argv[2]);
        if (m == 0) value_return = err_m_not_valid();
    }
    if (value_return == 0) {
        part = atoi(argv[1]);
        if (part >= m) value_return = err_part_not_valid();
    }

    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
        value_return = err_fcntl();
    }
    if (fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK)) {
        value_return = err_fcntl();
    }
    char str[15];
    sprintf(str, "%d.txt", getpid());
    FILE* debug = fopen(str, "a");
    fprintf(debug, "AVVIATO Q con m = %d part = %d\n", m, part);
    fclose(debug);

    while (value_return == 0 && !_close) {
        if (read(STDIN_FILENO, path, PATH_MAX) > 0) {  //Legge un percorso
            debug = fopen(str, "a");
            fprintf(debug, "Q: LEGGO %s\n", path);
            fclose(debug);
            if (!strncmp(path, "#CLOSE", 6)) {  //Se leggo una stringa di terminazione
                _close = TRUE;                  //Setto end a true
            } else {                            //Senno' analizzo il path
                tmpDup = strdup(path);
                id = strtok(tmpDup, "#");
                analyze = strtok(NULL, "#");
                fp = fopen(analyze, "r");
                if (fp == NULL) {
                    //value_return = err_file_open();
                    for (i = 0; i < DIM_V; i++) {
                        v[i] = -1;
                    }
                } else {
                    get_frequencies(fp, v, part, m);
                    fclose(fp);
                }
                createCsv(v, resp, id);
                debug = fopen(str, "a");
                fprintf(debug, "Q: ANALIZZATO %s \n", resp);
                fclose(debug);
                respSent = FALSE;
                while (!respSent) {  //finchè la risposta non è stata inviata riprova
                    if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {
                        if (errno != EAGAIN) {
                            value_return = err_write();
                        }
                    } else {
                        respSent = TRUE;
                        debug = fopen(str, "a");
                        fprintf(debug, "Q: RISCRIVO %s \n", resp);
                        fclose(debug);
                    }
                }
                free(tmpDup);
            }
        }
    }

    return value_return;
}