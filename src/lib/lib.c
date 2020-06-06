#include "lib.h"

void close_all_process() {
    fprintf(stderr, BOLDRED "\n[ERRORE]" RESET "\tIl programma verrà terminato a seguito di un errore inatteso.\n");
    int grp_id = getpgrp();
    kill(-grp_id, 9);
    exit(-1);
}

//Allocate memory for process* and process->pid
process *create_process(int size) {
    process *st = (process *)malloc(sizeof(process));  //Allocate process memory
    st->size = size;                                   //Set process->size to size (end point)
    st->pid = (int *)malloc(sizeof(int *) * size);     //Allocate pid memory
    st->count = 0;                                     //Set count to 0 (start point)
    int i;                                             //Declaring a new int
    for (i = 0; i < size; i++) {                       //for cycle from 0 to end point
        st->pid[i] = -1;                               //Set the default value (-1) to every pid in the list
    }
    return st;  //return process
}

void insertProcess(process *tmp, pid_t val) {                            //Insert process val in list tmp
    if (tmp->count == tmp->size) {                                       //If tmp->pid isn't enough big to contain another value
        int i;                                                           //Initialize i
        tmp->size *= 2;                                                  //We doubled the size
        tmp->pid = (int *)realloc(tmp->pid, sizeof(int *) * tmp->size);  //We realloc it's memory to double it
        for (i = tmp->count; i < tmp->size; i++) {                       //Cycle from old size to new size
            tmp->pid[i] = -1;                                            //Set pid[i] to -1 (from old size to new size)
        }
    }
    tmp->pid[tmp->count] = val;  //Insert the pid "val" in position count of tmp
    tmp->count++;                //Increase the count of variables inside
}

void freeList(process *tmp) {  //free the list tmp
    free(tmp->pid);            //free the array of pid
    free(tmp);                 //free the list tmp
}

void initialize_processes(pid_t *p, int dim) {
    int i;
    for (i = 0; i < dim; i++) {
        p[i] = -1;
    }
}

array *createPathList(int size) {                        //allocate an array for the PathList
    array *st = (array *)malloc(sizeof(array));          //allocate list of paths
    st->size = size;                                     //assign the size of the array
    st->pathList = malloc(sizeof(char **) * size);       //allocate the array of path (strings / char*)
    st->analyzed = (int *)malloc(sizeof(int *) * size);  //allocate the array of int analyzed
    st->count = 0;                                       //set the counter to 0
    st->last_edit = (time_t *)malloc(sizeof(time_t *) * size);
    int i;                                                              //initialize variable i for the next cycle
    for (i = 0; i < size; i++) {                                        //start the cycle from 0 to the size of the process
        st->pathList[i] = (char *)malloc(sizeof(char *) * (DIM_PATH));  //allocate the array of chars that compose the string
        memset(st->pathList[i], '\0', sizeof(char *) * DIM_PATH);       //set the first character to '\0' (= end of string)
        st->analyzed[i] = -1;                                           //set analyzed to -1 (= not analyzed)
    }
    return st;  //return the created list of paths
}

void reallocPathList(array *tmp, int newSize) {
    int i;
    tmp->size *= newSize;
    tmp->pathList = (char **)realloc(tmp->pathList, sizeof(char **) * tmp->size);
    tmp->analyzed = (int *)realloc(tmp->analyzed, sizeof(int *) * tmp->size);
    tmp->last_edit = (time_t *)realloc(tmp->last_edit, sizeof(time_t *) * tmp->size);

    for (i = tmp->count; i < tmp->size; i++) {
        tmp->pathList[i] = (char *)malloc(sizeof(char *) * (DIM_PATH));
        memset(tmp->pathList[i], '\0', sizeof(char *) * DIM_PATH);  //set the first character to '\0' (= end of string)
        tmp->analyzed[i] = -1;
    }
}

char insertPathList(array *tmp, char *c, int val) {
    int i;
    char present = FALSE;
    char ret = FALSE;
    char *dup1 = NULL;
    char *dup2 = NULL;
    char *compare1;
    char *compare2;
    struct stat attr;

    for (i = 0; i < tmp->count; i++) {
        dup1 = strdup(tmp->pathList[i]);
        dup2 = strdup(c);
        compare1 = strtok(dup1, "#");
        compare1 = strtok(NULL, "#");
        compare2 = strtok(dup2, "#");
        compare2 = strtok(NULL, "#");

        if (!strcmp(compare1, compare2)) {
            present = TRUE;
        }
        free(dup1);
        free(dup2);
    }
    if (present == FALSE) {
        if (tmp->count == tmp->size) {
            reallocPathList(tmp, 2);
        }
        dup2 = strdup(c);
        compare2 = strtok(dup2, "#");
        compare2 = strtok(NULL, "#");
        stat(compare2, &attr);
        tmp->last_edit[tmp->count] = attr.st_mtime;
        free(dup2);

        strcpy(tmp->pathList[tmp->count], c);
        tmp->analyzed[tmp->count] = val;
        tmp->count++;
        ret = TRUE;
    } else {
        ret = FALSE;
    }
    return ret;
}

int insertAndSumPathList(array *tmp, char *c, int val) {
    int i;
    char sum = FALSE;
    int ret = -1;

    for (i = 0; i < tmp->count; i++) {
        if (sameId(tmp->pathList[i], c)) {
            if (sumCsv(tmp->pathList[i], c)) {
                sum = TRUE;
                tmp->analyzed[i]--;
                if (tmp->analyzed[i] == 0) {
                    ret = i;
                }
            }
        }
    }

    if (sum == FALSE) {
        if (tmp->count == tmp->size - 1) {
            reallocPathList(tmp, 2);
        }
        strcpy(tmp->pathList[tmp->count], c);
        tmp->analyzed[tmp->count] = val;
        tmp->count++;
        ret = -1;
        if (val == 0) ret = tmp->count - 1;
    }
    return ret;
}

char removeFromPathList(array *tmp, char *c) {
    int i;
    char ret = FALSE;
    char *dup1 = NULL;
    char *dup2 = NULL;
    char *compare1;
    char *compare2;

    for (i = 0; i < tmp->count; i++) {
        dup1 = strdup(tmp->pathList[i]);
        dup2 = strdup(c);
        compare1 = strtok(dup1, "#");
        compare1 = strtok(NULL, "#");
        compare2 = strtok(dup2, "#");
        compare2 = strtok(NULL, "#");

        if (!strcmp(compare1, compare2)) {
            if (tmp->analyzed[i] != REMOVED) {
                tmp->analyzed[i] = REMOVED;
                ret = TRUE;
            }
        }
        free(dup1);
        free(dup2);
    }
    return ret;
}

void freePathList(array *tmp) {
    int i;
    for (i = 0; i < tmp->size; i++) {
        free(tmp->pathList[i]);
    }
    free(tmp->pathList);
    free(tmp->analyzed);
    free(tmp->last_edit);
    free(tmp);
}

array *resetPathList(array *tmp) {
    freePathList(tmp);
    return createPathList(10);
}

//Compare mtime of string1 and string2, returns TRUE if equal
int compare_mtime(array *tmp, int i, char *str) {
    int value_return = FALSE;
    struct stat attr;
    stat(str, &attr);
    if (tmp->last_edit[i] == attr.st_mtime) {
        value_return = TRUE;
    }

    return value_return;
}

//Esegue l'update dell'ultima modifica dei file a tutta la lista
void update_mtime(array *lista) {
    int i;
    struct stat attr;
    char *tmpPercorso;
    char *file;
    for (i = 0; i < lista->count; i++) {
        tmpPercorso = strdup(lista->pathList[i]);
        file = strtok(tmpPercorso, "#");
        file = strtok(NULL, "#");
        stat(file, &attr);
        lista->last_edit[i] = attr.st_mtime;
        free(tmpPercorso);
    }
}

void cleanRemoved(array *lista) {
    int j;
    char *dup = NULL;
    char *path = NULL;
    char newPath[DIM_PATH];

    array *tmp = createPathList(lista->size);
    for (j = 0; j < lista->count; j++) {
        if (lista->analyzed[j] != REMOVED) {
            insertPathList(tmp, lista->pathList[j], lista->analyzed[j]);
        }
    }
    freePathList(lista);
    lista = createPathList(tmp->count + 1);
    for (j = 0; j < tmp->count; j++) {
        dup = strdup(tmp->pathList[j]);
        path = strtok(dup, "#");
        path = strtok(NULL, "#");
        sprintf(newPath, "%d#%s", lista->count, path);
        insertPathList(lista, newPath, tmp->analyzed[j]);
        free(dup);
    }
    freePathList(tmp);
}
char sameId(char *a, char *b) {
    char ret = FALSE;
    char *dup1 = NULL;
    char *dup2 = NULL;
    dup1 = strdup(a);
    dup2 = strdup(b);
    char *id1 = strtok(dup1, "#");
    char *id2 = strtok(dup2, "#");
    if (!strcmp(id1, id2)) {
        ret = TRUE;
    }
    free(dup1);
    free(dup2);
    return ret;
}

int parser2(int argc, char *argv[], array *lista, int *count, int *n, int *m, int *res, int *duplicate) {
    char type;
    int i;
    FILE *fp;
    *res = 0;
    int conc = 0;
    char *tmp;
    int len;
    char riga[DIM_PATH - 16];
    char resolved_path[DIM_PATH - 16];
    int ret = parser_CheckArguments(argc, argv, &(*n), &(*m));
    *duplicate = 0;
    if (ret < 0) {
        ret = ERR_ARGS_A;
    } else if (ret > 0) {
        err_input_A(argv[ret]);
        printf("ERRORE\n");
        ret = ERR_ARGS_A;
    } else {
        if (!strncmp(argv[0], "add", 3) || !strncmp(argv[0], "./A", 3))
            type = ADD;
        else
            type = REMOVE;
        for (i = 1; i < argc && ret == 0; i++) {
            if (!strcmp(argv[i], "-setn") || !strcmp(argv[i], "-setm")) {
                i++;
            } else {
                conc = i;
                while (check_spaces(argv[i])) {
                    len = strlen(argv[i]);
                    tmp = strdup(argv[i]);
                    tmp[len - 1] = '\0';
                    strcat(argv[i], " ");
                    strcat(argv[i], argv[conc + 1]);
                    strcat(tmp, " ");
                    strcat(tmp, argv[conc + 1]);
                    conc++;
                }
                char command[parser_LenghtCommand(argv[i])];
                sprintf(command, "find %s -type f -follow -print", argv[i]);
                fp = popen(command, "r");  //avvia il comando e in fp prende l'output
                if (fp == NULL) {
                    ret = ERR_ARGS_A;
                    printf("Errore\n");
                } else {  //Il comando va a buon fine
                    while (fgets(riga, sizeof(riga), fp) != NULL) {
                        realpath(riga, resolved_path);                    //risalgo al percorso assoluto
                        resolved_path[strlen(resolved_path) - 1] = '\0';  //tolgo l'ultimo carattere che manderebbe a capo
                        char path[DIM_PATH];
                        sprintf(path, "%d#%s", lista->count, resolved_path);
                        if (type == ADD) {
                            if (insertPathList(lista, path, 0)) {
                                (*count)++;
                                (*res)++;
                            } else {
                                (*duplicate)++;
                            }
                        } else {
                            if (removeFromPathList(lista, path)) {
                                (*count)--;
                                (*res)++;
                            } else {
                                (*duplicate)++;
                            }
                        }
                    }
                }
                pclose(fp);
                i += conc - i;
                if (conc != i) {
                    free(tmp);
                }
            }
        }
    }
    return ret;
}

int parser_LenghtCommand(char *search) {
    int ret = strlen("find  -type f -follow -print") + strlen(search) + 1;
    return ret;
}

//Return -1 if ERR_ARGS, 0 if all arguments are correct, i if argv[i] doesn't exist
int parser_CheckArguments(int argc, char *argv[], int *n, int *m) {
    int ret = 0;
    char *tmp;
    int i;
    int conc = 0;
    int len;
    char setn = FALSE;  //se setn = true, n è stato cambiato
    char setm = FALSE;
    if (argc < 1) {  //if number of arguments is even or less than 1, surely it's a wrong input
        ret = -1;
    } else {
        for (i = 1; i < argc && ret == 0; i++) {
            if (!strcmp(argv[i], "-setn")) {  //ERRORI -setn
                if (i + 1 < argc) {           //controlla che ci sia effettivamente un argomento dopo il -setn
                    *n = atoi(argv[i + 1]);
                    if (*n == 0) ret = -1;       //Il campo dopo -setn non è un numero
                    if (setn == TRUE) ret = -1;  //n gia' settato
                    setn = TRUE;                 //n e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    ret = -1;
                }

            } else if (!strcmp(argv[i], "-setm")) {  //ERRORI -setm
                if (i + 1 < argc) {                  //controlla che ci sia effettivamente un argomento dopo il -setn
                    *m = atoi(argv[i + 1]);
                    if (*m == 0) ret = -1;       //Il campo dopo -setm non è un numero
                    if (setm == TRUE) ret = -1;  //m gia' settato
                    setm = TRUE;                 //m e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    ret = -1;
                }
            } else {
                conc = i;
                while (check_spaces(argv[i])) {
                    tmp = strdup(argv[i]);
                    len = strlen(tmp);
                    tmp[len - 1] = '\0';
                    strcat(argv[i], " ");
                    strcat(argv[i], argv[conc + 1]);
                    strcat(tmp, " ");
                    strcat(tmp, argv[conc + 1]);
                    conc++;
                }
                if (conc != i) {
                    if (!fileExist(tmp)) {
                        ret = i;
                    }
                } else {
                    if (!fileExist(argv[i])) {
                        ret = i;
                    }
                }
                i += conc - i;
                free(tmp);
            }
        }
    }
    return ret;
}

int check_spaces(char *str) {
    int len = strlen(str);
    int ret = FALSE;
    if (str[len - 1] == '\\') {
        ret = TRUE;
    }
    return ret;
}

void close_pipes(int *fd, int size) {
    int i;
    for (i = 0; i < size; i++) {
        close(fd[i]);
    }
}

int unlock_pipes(int *fd, int size) {
    int i;
    int ret = 0;
    for (i = 0; i < size && ret == 0; i++) {
        if (fcntl(fd[i], F_SETFL, O_NONBLOCK)) {
            ret = -1;
        }
    }
    return ret;
}

void forkC(int *n, int *f, int *id, int *value_return) {
    int i;
    for (i = 0; i<*n && * f> 0 && *value_return == 0; i++) {
        *f = fork();
        if (*f == 0) {
            *id = i;                     //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
        } else if (*f == -1) {           //Controllo che non ci siano stati errori durante il fork
            *value_return = err_fork();  //In caso di errore setta il valore di ritorno a ERR_FORK
        }
    }
}
void forkP(int *m, int *f, int *id, int *value_return) {
    int i;
    for (i = 0; i<*m && * f> 0 && *value_return == 0; i++) {
        *f = fork();
        if (*f == 0) {
            *id = i;                     //Assegno ad id il valore di i cosi' ogni figlio avra' un id diverso
        } else if (*f == -1) {           //Controllo che non ci siano stati errori durante il fork
            *value_return = err_fork();  //In caso di errore setta il valore di ritorno a ERR_FORK
        }
    }
}

void execC(int *m, int *f, int *id, int *fd, int *value_return, int *size_pipe) {
    char str[12];
    sprintf(str, "%d", *m);
    char *args[3] = {"./P", str, NULL};
    dup2(fd[*id * 4 + 2], STDIN_FILENO);
    dup2(fd[*id * 4 + 1], STDOUT_FILENO);
    close_pipes(fd, *size_pipe);
    free(fd);
    if (execvp(args[0], args) == -1) {    //Test exec
        *value_return = err_exec(errno);  //Set value return
    }
}
void execP(int *m, int *f, int *id, int *fd, int *value_return, int *size_pipe) {
    char str[12];
    sprintf(str, "%d", *id);
    char str2[12];
    sprintf(str2, "%d", *m);
    char *args[4] = {"./Q", str, str2, NULL};
    dup2(fd[*id * 4 + 2], STDIN_FILENO);
    dup2(fd[*id * 4 + 1], STDOUT_FILENO);
    close_pipes(fd, *size_pipe);
    free(fd);
    if (execvp(args[0], args) == -1) {    //Test exec
        *value_return = err_exec(errno);  //Set value return
    }
}

int createPipe(int *fd, int size_pipe) {
    int i;
    int ret = 0;
    for (i = 0; i < size_pipe - 1; i += 2) {
        if (pipe(fd + i) == -1) {  //Controlla se ci sono errori nella creazione della pipe
            ret = ERR_PIPE;        //In caso di errore setta il valore di ritorno
        }
    }
    return ret;
}

int setOnFly(int n, int m, int *fd_1) {
    int ret = 0;
    if (fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) {
        ret = err_fcntl();
    }
    char resp[DIM_RESP];
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    while (read(fd_1[READ], resp, DIM_RESP) > 0)
        ;
    char onFly[DIM_PATH];
    memset(onFly, '\0', DIM_PATH);
    sprintf(onFly, "#SET#%d#%d#", n, m);
    if (write(fd_1[WRITE], onFly, DIM_PATH) == -1) {  //Prova a scrivere sulla pipe
        if (errno != EAGAIN) {                        //Se avviene un errore e non e` causato dalla dimensione della pipe
            ret = err_write();
        }
    }
    return ret;
}

int setmOnFly(int m, int *fd_1) {
    int ret = 0;
    if (fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) {
        ret = err_fcntl();
    }
    char resp[DIM_RESP];
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    while (read(fd_1[READ], resp, DIM_RESP) > 0)
        ;
    char onFly[DIM_PATH];
    memset(onFly, '\0', DIM_PATH);
    sprintf(onFly, "#SETM#%d#", m);
    if (write(fd_1[WRITE], onFly, DIM_PATH) == -1) {  //Prova a scrivere sulla pipe
        if (errno != EAGAIN) {                        //Se avviene un errore e non e` causato dalla dimensione della pipe
            ret = err_write();                        //Ritorna l'errore sulla scrittura
        } else
            fprintf(stderr, "errore set on-fly, pipe piena\n");
    }
    return ret;
}

void mParseOnFly(char *path, int *m) {
    char *token;
    char *dupPath = strdup(path);
    token = strtok(dupPath, "#");
    token = strtok(NULL, "#");
    *m = atoi(token);
    free(dupPath);
}

void parseOnFly(char *path, int *n, int *m) {
    char *token;
    char *dupPath = strdup(path);
    token = strtok(path, "#");
    token = strtok(NULL, "#");
    *n = atoi(token);
    token = strtok(NULL, "#");
    *m = atoi(token);
    free(dupPath);
}

int parseSetOnFly(char *string, int *n, int *m) {
    int tmpn;
    int tmpm;
    int ret = 0;
    char *token;
    char *dupPath = strdup(string);
    token = strtok(dupPath, " ");
    token = strtok(NULL, " ");
    tmpn = atoi(token);
    token = strtok(NULL, " ");
    tmpm = atoi(token);
    if ((tmpn == (*n) && tmpm == (*m)) || (tmpn == 0 || tmpm == 0)) {
        ret = -1;
    } else {
        *n = tmpn;
        *m = tmpm;
    }

    free(dupPath);
    return ret;
}

int nClearAndClose(int *fd, int n) {
    int i;
    char sentClose = FALSE;
    char path[DIM_PATH];
    memset(path, '\0', sizeof(char) * DIM_PATH);
    int ret = 0;
    int terminated[n];  //Indica se un file e` stato mandato o meno
    for (i = 0; i < n; i++) {
        if (fcntl(fd[i * 4 + 2], F_SETFL, O_NONBLOCK)) {
            ret = err_fcntl();
        }
        while (read(fd[i * 4 + 2], path, DIM_PATH) > 0) {
        }
        terminated[i] = FALSE;
    }
    strcpy(path, "#CLOSE");
    while (!sentClose) {
        sentClose = TRUE;
        for (i = 0; i < n; i++) {  //Provo a inviare path a tutti i Q
            if (!terminated[i]) {
                if (write(fd[i * 4 + 3], path, DIM_PATH) == -1) {
                    if (errno != EAGAIN) {
                        ret = err_write();
                    } else {
                        sentClose = FALSE;  //Se non ci riesce setta send a false
                        terminated[i] = FALSE;
                    }
                } else {
                    terminated[i] = TRUE;
                }
            }
        }
    }

    return ret;
}

int mSendOnFly(int *fd, int n, int m) {
    int i;
    int ret = 0;
    char sentClose = FALSE;
    char path[DIM_PATH];
    memset(path, '\0', sizeof(char) * DIM_PATH);
    int terminated[n];  //Indica se un file e` stato mandato o meno
    for (i = 0; i < n; i++) {
        if (fcntl(fd[i * 4 + 2], F_SETFL, O_NONBLOCK)) {
            ret = err_fcntl();
        }
        while (read(fd[i * 4 + 2], path, DIM_PATH) > 0) {
        }
        terminated[i] = FALSE;
    }
    sprintf(path, "#SETM#%d#", m);
    //fprintf(stderr,"%s\n",path);
    while (!sentClose) {
        sentClose = TRUE;
        for (i = 0; i < n; i++) {  //Provo a inviare path a tutti i Q
            if (!terminated[i]) {
                if (write(fd[i * 4 + 3], path, DIM_PATH) == -1) {
                    if (errno != EAGAIN) {
                        ret = err_write();
                    } else {
                        sentClose = FALSE;  //Se non ci riesce setta send a false
                        terminated[i] = FALSE;
                    }
                } else {
                    terminated[i] = TRUE;
                }
            }
        }
    }
    return ret;
}

int closeAll(int *fd_1) {
    int ret = 0;
    if (fcntl(fd_1[READ], F_SETFL, O_NONBLOCK)) {
        ret = err_fcntl();
    }
    char path[DIM_PATH];
    while (read(fd_1[READ], path, DIM_PATH) > 0)
        ;
    if (write(fd_1[WRITE], "#CLOSE", DIM_PATH) == -1) {  //Prova a scrivere sulla pipe
        if (errno != EAGAIN) {                           //Se avviene un errore e non e` causato dalla dimensione della pipe
            ret = err_write();                           //Ritorna l'errore sulla scrittura
        }
    }

    return ret;
}

void nCleanSon(int *fd, int n) {
    char cleanPipe = FALSE;
    int son = 0;
    cleanPipe = FALSE;
    char resp[DIM_RESP];
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    while (!cleanPipe) {  //Ciclo per svuotare tutte le pipe in lettura da Q a P
        cleanPipe = TRUE;
        for (son = 0; son < n; son++) {  //Cicla tra tutti i figli
            if (read(fd[son * 4 + 0], resp, DIM_RESP) > 0) {
                cleanPipe = FALSE;
            }
        }
    }
}

char sendCheck() {
    char resp[DIM_RESP];
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    strcpy(resp, "#CHECK");
    char CheckSent = FALSE;
    char ret = TRUE;
    while (!CheckSent && ret == TRUE) {
        if (write(STDOUT_FILENO, resp, DIM_RESP) == -1) {  //Scrive il carattere di teminazione
            if (errno != EAGAIN) {
                ret = FALSE;
            }
        } else {
            CheckSent = TRUE;
        }
    }
    return ret;
}

void readCheck(int *fd, int n) {
    int recived = 0;
    int k;
    char resp[DIM_RESP];
    memset(resp, '\0', sizeof(char) * DIM_RESP);
    if (n == 0) {
        n = 1;
        while (recived < n) {
            if (read(fd[READ], resp, DIM_RESP) > 0) {  //Prova a leggere dal figlio
                if (!strncmp(resp, "#CHECK", 6)) {
                    recived++;
                }
            }
        }
    } else {
        while (recived < n) {
            if (read(fd[k * 4 + 0], resp, DIM_RESP) > 0) {  //Prova a leggere dalla pipe
                if (!strncmp(resp, "#CHECK", 6)) {
                    recived++;
                }
            }
            k = (k + 1) % n;
        }
    }
}

int parse_string(char *string, int v[DIM_V]) {
    int i = 0;
    int max = 0;
    char *token = strtok(string, ",");
    while (token != NULL) {  //loop through token
        if (i < DIM_V) {
            v[i] = atoi(token);
            if (v[i] > max)
                max = v[i];
            i++;
            token = strtok(NULL, ",");
        } else {
            printf("Errore nella creazione della stringa contatore");
        }
    }
    return max;
}

void initialize_vector(long *v) {
    int i;
    for (i = 0; i < DIM_V; i++) {
        v[i] = 0;
    }
}

void set_add(long *v, char c) {
    int val_ascii;
    val_ascii = ((int)c) - 32;  //casting char to int and difference 32 (in order to save space on the vector) //Se vogliamo togliere lo spazio basta fare -33
    if (val_ascii >= 0) {
        v[val_ascii]++;
    }
}

void get_subset(int *fp, long *v, int b, int e) {
    int i = b;
    char c[1];
    lseek(*fp, b, SEEK_SET);  //setting initial position of SEEK cursor
    while (read(*fp, c, 1) && i < e) {
        if (c[0] != '\n') set_add(v, c[0]);
        i++;
    }
}

void get_frequencies(int *fp, long *freq, int part, int m) {  //Questa funzione e` in O(1)
    initialize_vector(freq);                                  //Inizializza il vettore delle frequenze
    int file_length = lseek(*fp, 0, SEEK_END);                //Salva la lunghezza totale del file
    int char_parts = file_length / m;                         //Conta di quanti caratteri deve essere ogni parte (floor round)
    int remain = file_length - (char_parts * m);              //Resto della divisione precedente -> utilizzato per correggere i caratteri per parte
    int begin = part * char_parts;                            //Calcola l'inizio con parte per numero di caratteri per parte (es. part = 2, char_parts = 3, begin = 6) 000 111 222
    int end = begin + char_parts;                             //La fine e` l'inizio piu` il numero di caratteri per parte
    if (remain > part) {                                      //Redistribuisce i caretteri in piu` sui primi "remain" parti, percio` se la parte e` minore di remain deve ricevere caratteri aggiuntivi
        begin += part;                                        //Avanza di un carattere per il numero della parte
        end += part + 1;                                      //Avanza di un carattere per il numero della parte + 1
    } else {                                                  //Caso in cui la parte non debba ricevere caratteri aggiuntivi
        begin += remain;                                      //Avanza di remain
        end += remain;                                        //Avanza di remain
    }
    get_subset(fp, freq, begin, end);
}

void arrayToCsv(long *v, char *res) {
    int i;
    char str[12];
    for (i = 0; i < DIM_RESP; i++) {
        res[i] = '\0';
    }
    sprintf(str, "%ld", v[0]);
    strcat(res, str);
    for (i = 1; i < DIM_V; i++) {
        strcat(res, ",");
        sprintf(str, "%ld", v[i]);
        strcat(res, str);
    }
}

int countDigit(int n) {
    int count = 0;
    if (n == 0) {
        count++;
    } else {
        while (n != 0) {
            n = n / 10;
            ++count;
        }
    }
    return count;
}

int lenghtCsv(int v[DIM_V]) {
    int i;
    int dim = 0;
    for (i = 0; i < DIM_V; i++) {
        dim += countDigit(v[i]);
    }
    dim += 95;
    return dim;
}

char sumCsv(char *str1, char *str2) {
    char ret = FALSE;
    int i;
    char *dup1 = NULL;
    char *dup2 = NULL;
    dup1 = strdup(str1);
    dup2 = strdup(str2);
    char *id1 = strtok(dup1, "#");
    char *tmp1 = strtok(NULL, "#");

    char *id2 = strtok(dup2, "#");
    char *tmp2 = strtok(NULL, "#");
    long v[DIM_RESP];
    initialize_vector(v);
    char deleteCheck = FALSE;
    int firstVal = 0;
    if (!strcmp(id1, id2)) {
        addCsvToArray(tmp1, v);
        firstVal = v[0];
        if (v[0] < 0) deleteCheck = TRUE;
        addCsvToArray(tmp2, v);
        if (v[0] - firstVal < 0) deleteCheck = TRUE;
        if (deleteCheck) {
            for (i = 0; i < DIM_V; i++) {
                v[i] = -1;
            }
        }
        createCsv(v, str1, id1);
        ret = TRUE;
    }
    free(dup1);
    free(dup2);
    return ret;
}

void createCsv(long *v, char *res, char *id) {
    int i;
    char str[12];
    for (i = 0; i < DIM_RESP; i++) {
        res[i] = '\0';
    }

    strcat(res, id);
    strcat(res, "#");
    sprintf(str, "%ld", v[0]);
    strcat(res, str);
    for (i = 1; i < DIM_V; i++) {
        strcat(res, ",");
        sprintf(str, "%ld", v[i]);
        strcat(res, str);
    }
    strcat(res, "#");
}

char addCsvToArray(char *tmp, long *v) {
    char ret = FALSE;
    int i = 0;
    char *token = strtok(tmp, "#");
    token = strtok(tmp, ",");
    while (token != NULL) {  //loop through token
        if (i < DIM_V) {
            v[i] += atoi(token);
            if (v[i] < 0) ret = TRUE;
            i++;
            token = strtok(NULL, ",");
        } else {
            printf("Errore nella creazione della stringa contatore");
        }
    }
    return ret;
}

char fileExist(char *fname) {
    char ret = FALSE;
    if (access(fname, F_OK) != -1) ret = TRUE;
    return ret;
}

char checkArg(char cmd[DIM_CMD], int *argCounter) {
    int j;
    char ret = TRUE;
    if (!(strstr(cmd, "  ") != NULL)) {
        *argCounter = 1;
        for (j = 0; j < DIM_CMD; j++) {
            //printf("%d\n", cmd[j]);
            if (cmd[j] == ' ') {
                (*argCounter)++;
            }
            if (cmd[j] == '\0') {
                if (cmd[j - 1] == ' ')
                    ret = FALSE;
                else
                    j = DIM_CMD - 1;
            }
        }
    } else {
        ret = FALSE;
    }
    return ret;
}

void printStat(char *char_count) {
    int v[DIM_V];
    int i, k;
    int column = 9;
    int max = parse_string(char_count, v);
    printf("STAMPA STATISTICHE GENERALI\n\n");
    if (max <= 9999999) {
        for (i = 0; i < DIM_V; i += column) {
            for (k = i; k < i + column && k < DIM_V; k++) {
                printf("%c\t", k + 32);
            }
            printf("\n");
            for (k = i; k < i + column && k < DIM_V; k++) {
                printf("%d\t", v[k]);
            }
            printf("\n\n");
        }
    } else {
        column = 5;
        for (i = 0; i < DIM_V; i += column) {
            for (k = i; k < i + column && k < DIM_V; k++) {
                printf("%c\t\t", k + 32);
            }
            printf("\n");
            for (k = i; k < i + column && k < DIM_V; k++) {
                printf("%d\t", v[k]);
                if (v[k] < 9999999) printf("\t");
            }
            printf("\n\n");
        }
    }
}

void analyzeCluster(char *stringa, char *resp) {
    char *char_count = strdup(stringa);
    int v[DIM_V];
    int i;
    int lettereMin = 0;
    int lettereMai = 0;
    int spazi = 0;
    int numeri = 0;
    int punt = 0;
    int carSpec = 0;
    int tot = 0;
    for (i = 0; i < DIM_V; i++) {
        v[i] = 0;
    }
    parse_string(char_count, v);
    free(char_count);
    for (i = 0; i < DIM_V; i++) {
        if (i == 0) {
            spazi += v[i];
            tot += v[i];
        }
        if (i == 1 || i == 2 || (i >= 7 && i <= 9) || (i >= 12 && i <= 15) || i == 26 || i == 27 || i == 31) {
            punt += v[i];
            tot += v[i];
        }
        if ((i >= 3 && i <= 6) || i == 10 || i == 11 || (i >= 28 && i <= 30) || i == 32 || (i >= 59 && i <= 64) || (i >= 91 && i <= 94)) {
            carSpec += v[i];
            tot += v[i];
        }
        if (i >= 16 && i <= 25) {
            numeri += v[i];
            tot += v[i];
        }
        if (i >= 33 && i <= 58) {
            lettereMai += v[i];
            tot += v[i];
        }
        if (i >= 65 && i <= 90) {
            lettereMin += v[i];
            tot += v[i];
        }
    }

    sprintf(resp, "%d,%d,%d,%d,%d,%d,%d", tot, spazi, punt, carSpec, numeri, lettereMai, lettereMin);
}

void printCluster(char *char_count) {
    int v[7];
    int i;
    for (i = 0; i < 7; i++) {
        v[i] = 0;
    }
    parse_string(char_count, v);
    int lettereMin = v[6];
    int lettereMai = v[5];
    int spazi = v[1];
    int numeri = v[4];
    int punt = v[2];
    int carSpec = v[3];
    int tot = v[0];

    printf(BOLDWHITE "STAMPA STATISTICHE PER CLUSTER\n\n" RESET);
    printf(WHITE "Lettere minuscole" RESET ":\t %d\t%.4g%%\n", lettereMin, (float)lettereMin / (float)tot * 100);
    printf(WHITE "Lettere maiuscole" RESET ":\t %d\t%.4g%%\n", lettereMai, (float)lettereMai / (float)tot * 100);
    printf(WHITE "Spazi" RESET ":\t\t\t %d\t%.4g%%\n", spazi, (float)spazi / (float)tot * 100);
    printf(WHITE "Numeri" RESET ":\t\t\t %d\t%.4g%%\n", numeri, (float)numeri / (float)tot * 100);
    printf(WHITE "Segni di punteggiatura" RESET ":\t %d\t%.4g%%\n", punt, (float)punt / (float)tot * 100);
    printf(WHITE "Caratteri speciali" RESET ":\t %d\t%.4g%%\n", carSpec, (float)carSpec / (float)tot * 100);
    printf("\n");
    printf(WHITE "Caratteri totali" RESET ": %d\n", tot);
}

void printInfoCluster() {
    int i;
    printf(WHITE "Lettere minuscole:\t" RESET);
    for (i = 65; i <= 90; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf(WHITE "Lettere maiuscole:\t" RESET);
    for (i = 33; i <= 58; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf(WHITE "Spazi:\t " RESET);
    printf("\n");
    printf(WHITE "Numeri:\t\t\t" RESET);
    for (i = 16; i <= 25; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf(WHITE "Segni di punteggiatura:\t" RESET);
    for (i = 1; i <= 31; i++) {
        if (i == 1 || i == 2 || (i >= 7 && i <= 9) || (i >= 12 && i <= 15) || i == 26 || i == 27 || i == 31) {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
    printf(WHITE "Caratteri speciali:\t" RESET);
    for (i = 3; i <= 94; i++) {
        if ((i >= 3 && i <= 6) || i == 10 || i == 11 || (i >= 28 && i <= 30) || i == 32 || (i >= 59 && i <= 64) || (i >= 91 && i <= 94)) {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
}

void printHelp() {
    printf(BOLDWHITE "\nLISTA COMANDI DISPONIBILI\n" RESET);
    printf(BOLDWHITE "add </path1> </path2>" RESET ": aggiunge uno o piu` file e/o una o piu` directory\n");
    printf(BOLDBLACK "\t es: add ../src ../deploy.sh\n" RESET);
    printf(BOLDWHITE "remove </path1> </path2>" RESET ": rimuove uno o piu` file e/o una o piu` directory\n");
    printf(BOLDBLACK "\t es: remove ../src ../deploy.sh\n" RESET);
    printf(BOLDWHITE "reset" RESET ": elimina dalla cache del programma le statistiche e tutti i percorsi analizzati e non\n");
    printf(BOLDWHITE "print" RESET " <-flag>\n");
    printf(WHITE "\tNessun flag" RESET ": stampa tutti i file inseriti\n");
    printf(WHITE "\t-d" RESET ": stampa tutti i file che dall'ultima analisi sono risultati inesistenti\n");
    printf(WHITE "\t-x" RESET ": stampa tutti i file che sono stati analizzati\n");
    printf(BOLDWHITE "analyze" RESET ": analizza tutti i file non ancora analizzati\n");
    printf(BOLDWHITE "reanalyze" RESET ": analizza tutti i file anche quelli gia` analizzati\n");
    printf(BOLDWHITE "stat" RESET ": durante l'analisi stampa a video lo stato di avanzamento\n");
    printf(BOLDWHITE "set <n> <m>" RESET ": setta i nuovi valori di n e m\n");
    printf(BOLDBLACK "\t es: set 4 5\n" RESET);
    printf(BOLDWHITE "setn <val>" RESET ": setta i nuovi valori di n\n");
    printf(BOLDBLACK "\t es: setn 4\n" RESET);
    printf(BOLDWHITE "setm <val>" RESET ": setta i nuovi valori di m\n");
    printf(BOLDBLACK "\t es: setm 5\n" RESET);
    printf(BOLDWHITE "report" RESET " <-flag>\n");
    printf(WHITE "\t-c" RESET ": stampa le statistiche per cluster\n");
    printf(WHITE "\t-a" RESET ": stampa la frequenza di ogni carattere\n");
    printf(BOLDWHITE "proc" RESET ": stampa a video il numero di processi totali\n");
    printf(BOLDWHITE "info" RESET ": mostra informazioni aggiuntive sul programma\n");
    printf(BOLDWHITE "close" RESET ": chiude il programma\n\n> ");
    fflush(stdout);
}

int printInfo() {
    int fptr;
    int ret = 0;
    char c[1];
    fptr = open("../README.txt", O_RDONLY);  //Apre il file di README in sola lettura
    printf("\n");
    if (fptr != -1) {                            //Se non ci sono stati errori
        while (read(fptr, c, 1)) putchar(c[0]);  //Stampa del README.txt
        close(fptr);                             //Chiude il file
    } else {
        ret = err_file_open();
    }
    printf(BOLDWHITE "\nCRITERI DI CLUSTERING\n" RESET);
    printInfoCluster();  //Stampa il criteri con il quale viene eseguito il clustering
    printf("\n> ");
    fflush(stdout);

    return ret;
}

//Error handlers
int err_pipe() {
    fprintf(stderr, "Errore nella creazione della pipe\n");
    return ERR_PIPE;
}

int err_args_A() {
    fprintf(stderr, "\nErrore nella sintassi del comando.\nUsa help per vedere la lista di comandi utilizzabili.\n\n");
    return ERR_ARGS_A;
}

int err_overflow() {
    fprintf(stderr, "\nErrore overflow\nI risultati delle analisi sono troppo grandi\nProva con meno file o con file meno pesanti\n\n");
    return ERR_ARGS_A;
}

int err_input_A(char *file) {
    fprintf(stderr, BOLDRED "\n[ERROR]" RESET " File/Directory non esistente: %s\n", file);
    return ERR_ARGS_A;
}

int err_file() {
    fprintf(stderr, "Errore, nessun file inserito\n");
    return ERR_FILE;
}
//Error if 3 arguments are inserted //Refers to Q.c
int err_fork() {
    fprintf(stderr, "Errore, fork non riuscito\n");
    return ERR_FORK;
}

int err_write() {
    fprintf(stderr, "Errore, write non riuscita\n");
    return ERR_WRITE;
}
//Error if fopen has failed //Refers to Q.c
int err_file_open() {
    fprintf(stderr, "[!] Errore nell'apertura del file\n");
    return ERR_FILE;
}

int err_fcntl() {
    fprintf(stderr, "Errore, sblocco pipe non riuscito\n");
    return ERR_FCNTL;
}

int err_exec(int err) {
    fprintf(stderr, "Errore nell'esecuzione di exec(%d) in: %d\n", err, getpid());
    return ERR_EXEC;
}

int err_fifo() {
    fprintf(stderr, "Errore nella creazione della pipe fifo\n");
    return ERR_FIFO;
}

int err_close() {
    fprintf(stderr, "Errore nella chiusura del file\n");
    return ERR_CLOSE;
}

int err_kill_process() {
    return ERR_KILL_PROC;
}