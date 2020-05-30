#include "../lib/lib.h"
int value_return = 0;

void sig_term_handler(int signum, siginfo_t* info, void* ptr) {
    value_return = err_kill_process();
    close_all_process();
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

    //Arguments passed
    int part;
    int m;
    long v[DIM_V];

    int fp;
    char* id;
    char* analyze;
    int i;
    char* tmpDup = NULL;

    //IPC Arguments
    char path[DIM_PATH];
    int _close = FALSE;
    char respSent = FALSE;
    char resp[DIM_RESP];
    int oldfl;
    int pendingPath = 0;

    //Parsing Arguments--------------------------------------------------------------------
    m = atoi(argv[2]);
    part = atoi(argv[1]);

    if (fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK)) {
        value_return = err_fcntl();
    }
       char str[15];
    sprintf(str, "Q%d.txt", getpid());
    FILE* debug = fopen(str, "a");
    fprintf(debug, "AVVIATO Q con m = %d part = %d\n", m, part);
    fclose(debug);

    while (value_return == 0 && !_close) {
        if (read(STDIN_FILENO, path, DIM_PATH) > 0) {  //Legge un percorso
            pendingPath++;
            if (pendingPath == 1) {
                if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
                    value_return = err_fcntl();
                }
            }
            debug = fopen(str, "a");
            fprintf(debug, "Q: LEGGO %s\n", path);
            fclose(debug);
            if (!strncmp(path, "#CLOSE", 6)) {  //Se leggo una stringa di terminazione
                _close = TRUE;                  //Setto end a true
            } else {                            //Senno' analizzo il path
                tmpDup = strdup(path);
                id = strtok(tmpDup, "#");
                analyze = strtok(NULL, "#");
                fp = open(analyze, O_RDONLY);
                if (fp == -1) {
                    for (i = 0; i < DIM_V; i++) {
                        v[i] = -1;
                    }
                } else {
                    get_frequencies(&fp, v, part, m);
                    close(fp);
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
                        pendingPath--;
                        debug = fopen(str, "a");
                        fprintf(debug, "Q: RISCRIVO %s \n", resp);
                        fclose(debug);
                    }
                }
                free(tmpDup);
            }
        }
        if (pendingPath == 0) {
            oldfl = fcntl(STDIN_FILENO, F_GETFL);
            if (oldfl == -1) {
                value_return = err_fcntl();
            }
            fcntl(STDIN_FILENO, F_SETFL, oldfl & ~O_NONBLOCK);
        }
    }
    while (read(STDOUT_FILENO, resp, DIM_RESP) > 0)
        ;
    if (value_return != 0) {
        close_all_process();
    }

    return value_return;
}