//Updated to 2020.05.02
#include "../lib/lib.h"
int value_return = 0;

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    value_return = err_kill_process_Q();
}

void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char* argv[]) {
    catch_sigterm();
    sleep(10);
    //Arguments passed
    int part;
    int m;
    int v[DIM_V];
    
    FILE* fp;
    char* id;
    char* analyze;
    int i;
    char* tmpDup = NULL;

    //IPC Arguments
    char path[PATH_MAX];
    int _write = FALSE;
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
    while (value_return == 0 && !_write) {
        if (read(STDIN_FILENO, path, PATH_MAX) > 0) {  //Legge un percorso
            if (!strncmp(path, "///", 3)) {            //Se e' terminazione allora setta write a true e rimando indietro
                _write = TRUE;
                respSent = FALSE;
                while (!respSent) {  //finchè la risposta non è stata inviata riprova
                    if (write(STDOUT_FILENO, path, DIM_RESP) == -1) {
                        if (errno != EAGAIN) {
                            value_return = err_write();
                        }
                    } else {
                        respSent = TRUE;
                    }
                }

            } else {  //Senno' analizzo il path
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
                respSent = FALSE;
                while (!respSent) {  //finchè la risposta non è stata inviata riprova
                    if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {
                        if (errno != EAGAIN) {
                            value_return = err_write();
                        }
                    } else {
                        respSent = TRUE;
                    }
                }
                free(tmpDup);
            }
        }
    }

    return value_return;
}