#include "lib.h"

//Allocate memory for process* and process->pid
process *create_process(int size) {
    process *st = (process *)malloc(sizeof(process));  //Allocate process memory
    st->size = size;                                   //Set process->size to size (end point)
    st->pid = (int *)malloc(sizeof(int *) * size);     //Allocate pid memory
    st->count = 0;                                     //Set count to 0 (start point)
    int i;                                             //Declaring a new int
    for (i = 0; i < size; i++)                         //for cycle from 0 to end point
    {                                                  //
        st->pid[i] = -1;                               //Set the default value (-1) to every pid in the list
    }                                                  //
    //printf("Lista allocata\n");
    return st;  //return process
}

void insertProcess(process *tmp, pid_t val) {                            //Insert process val in list tmp
    if (tmp->count == tmp->size)                                         //If tmp->pid isn't enough big to contain another value
    {                                                                    //
        int i;                                                           //Initialize i
        tmp->size *= 2;                                                  //We doubled the size
        tmp->pid = (int *)realloc(tmp->pid, sizeof(int *) * tmp->size);  //We realloc it's memory to double it
        for (i = tmp->count; i < tmp->size; i++) {                       //Cycle from old size to new size
            tmp->pid[i] = -1;                                            //Set pid[i] to -1 (from old size to new size)
        }                                                                //
    }                                                                    //
    tmp->pid[tmp->count] = val;                                          //Insert the pid "val" in position count of tmp
    tmp->count++;                                                        //Increase the count of variables inside
}

void printList(process *tmp) {  //ONLY FOR TESTING -- NOT FOR PROJECT -- use it to print the process saved in tmp
    int i;
    for (i = 0; i < tmp->count; i++) {
        printf("%d: A=%d\n", i, tmp->pid[i]);
    }
}

void freeList(process *tmp) {  // free the list tmp
    free(tmp->pid);            // free the array of pid
    free(tmp);                 // free the list tmp
}  //

array *createPathList(int size) {                                       //allocate an array for the PathList
    array *st = (array *)malloc(sizeof(array));                         //allocate list of paths
    st->size = size;                                                    //assign the size of the array
    st->pathList = malloc(sizeof(char **) * size);                      //allocate the array of path (strings / char*)
    st->analyzed = (int *)malloc(sizeof(int *) * size);                 //allocate the array of int analyzed
    st->count = 0;                                                      //set the counter to 0
    int i;                                                              //initialize variable i for the next cycle
    for (i = 0; i < size; i++)                                          //start the cycle from 0 to the size of the process
    {                                                                   //
        st->pathList[i] = (char *)malloc(sizeof(char *) * (PATH_MAX));  //allocate the array of chars that compose the string
        memset(st->pathList[i], '\0', sizeof(char *) * PATH_MAX);       //set the first character to '\0' (= end of string)
        st->analyzed[i] = -1;                                           //set analyzed to -1 (= not analyzed)
    }                                                                   //
    return st;                                                          //return the created list of paths
}

char insertPathList(array *tmp, char *c, int val) {
    int i;
    char present = FALSE;
    char ret = FALSE;
    for (i = 0; i < tmp->count; i++) {
        if (!strcmp(tmp->pathList[i], c)) {
            present = TRUE;
        }
    }
    if (present == FALSE) {
        //printf("Provo a inserire %s, size: %d, count:%d\n", c, tmp->size, tmp->count);
        if (tmp->count == tmp->size) {
            reallocPathList(tmp, 2);
        }
        //printf("Stringa inserita\n");
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
        //printf("Provo a inserire %s, size: %d, count:%d\n", c, tmp->size, tmp->count);
        if (tmp->count == tmp->size - 1) {
            reallocPathList(tmp, 2);
        }
        //printf("Stringa inserita\n");
        strcpy(tmp->pathList[tmp->count], c);
        tmp->analyzed[tmp->count] = val;
        tmp->count++;
        ret = -1;
    }
    return ret;
}

void reallocPathList(array *tmp, int newSize) {
    int i;
    tmp->size *= newSize;
    tmp->pathList = (char **)realloc(tmp->pathList, sizeof(char **) * tmp->size);
    tmp->analyzed = (int *)realloc(tmp->analyzed, sizeof(int *) * tmp->size);

    for (i = tmp->count; i < tmp->size; i++) {
        tmp->pathList[i] = (char *)malloc(sizeof(char *) * (DIM_RESP + 1));
        //memset(tmp->pathList[i], '\0', sizeof(char *) * PATH_MAX);  //set the first character to '\0' (= end of string)
        tmp->analyzed[i] = -1;
    }
}

void printPathList(array *tmp) {
    int i;
    for (i = 0; i < tmp->count; i++) {
        fprintf(stderr, "%d: A=%d %s\n", i, tmp->analyzed[i], tmp->pathList[i]);
    }
}

int dimPathList(array *tmp) {
    return tmp->count;
}

void freePathList(array *tmp) {
    int i;
    for (i = 0; i < tmp->size; i++) {
        free(tmp->pathList[i]);
    }
    free(tmp->pathList);
    free(tmp->analyzed);
    free(tmp);
}

void setAnalyzed(array *tmp, int pos, int value) {
    if (pos >= tmp->count) {
        printf("Errore, il file da analizzare non e` presente in questa posizione\n");
    } else if (value > -1 && value < 3) {
        tmp->analyzed[pos] = value;
    } else {
        printf("Errore, valore assegnato al percorso non valido\n");
    }
}

int getAnalyzed(array *tmp, int pos) {
    return tmp->analyzed[pos];  //-1 if not analyzed
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

void close_pipes(int *fd, int size) {
    int i;
    for (i = 0; i < size; i++) {
        close(fd[i]);
    }
}
char sameId(char *a, char *b) {
    char *id1 = strtok(strdup(a), "#");
    char *id2 = strtok(strdup(b), "#");
    return (!strcmp(id1, id2));
}

// /src/Analyzer/A.c
// Handler for SIGINT, caused by
// Ctrl-C at keyboard
int parser(int argc, char *argv[], array *lista, int *count, int *n, int *m) {
    int value_return = 0;
    int i;
    char resolved_path[PATH_MAX];  //contiene il percorso assoluto di un file
    char flag = FALSE;             // se flag = true, non bisogna analizzare l'argomento. (l'argomento successivo è il numero o di n o di m)
    char setn = FALSE;             // se setn = true, n è stato cambiato
    char setm = FALSE;             // se setn = true, m è stato cambiato
    char errdir = FALSE;
    FILE *fp;
    char riga[1035];

    if (argc < 1) {  //if number of arguments is even or less than 1, surely it's a wrong input
        value_return = err_args_A();
    } else {
        for (i = 1; i < argc && value_return == 0; i++) {
            //printf("Argv: %s\n",argv[i]);
            if (!strcmp(argv[i], "-setn")) {  //----ERRORI -setn
                if (i + 1 < argc) {           //controlla che ci sia effettivamente un argomento dopo il -setn
                    *n = atoi(argv[i + 1]);
                    if (*n == 0) value_return = err_args_A();       //Il campo dopo -setn non è un numero
                    if (setn == TRUE) value_return = err_args_A();  //n gia' settato
                    flag = TRUE;                                    //Salto prossimo argomento
                    setn = TRUE;                                    //n e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    value_return = err_args_A();
                }

            } else if (!strcmp(argv[i], "-setm")) {  //-----ERRORI -setm
                if (i + 1 < argc) {                  //controlla che ci sia effettivamente un argomento dopo il -setn
                    *m = atoi(argv[i + 1]);
                    if (*m == 0) value_return = err_args_A();       //Il campo dopo -setm non è un numero
                    if (setm == TRUE) value_return = err_args_A();  //m gia' settato
                    flag = TRUE;                                    //Salto il prossimo argomento
                    setm = TRUE;                                    //m e' stato settato, serve a controllare che non venga settato due volte
                    i++;
                } else {
                    value_return = err_args_A();
                }

            }  //else if(strncmp(argv[i], "-", 1) == 0){//-----ERRORI input strani che iniziano con -
            //    value_return = err_args_A();
            //}

            if (flag == FALSE && value_return == 0) {  //------Vuol dire che argv[i] e' un file o una cartella
                /*  Viene utilizzato il comando:  test -d [dir] && find [dir] -type f -follow -print || echo "-[ERROR]"
                    Il comando test -d controlla l'esistenza del file/directory input. In caso di successo viene lanciato find
                    In caso di successo viene lanciato find che restituisce la lista di tutti i file nella cartella e nelle sottocartelle
                    Se l'input non esiste restituisce -[ERROR], in modo che possa essere intercettato dal parser
                */

                char command[strlen("(test -f  || test -d ) && find ") + strlen(argv[i]) * 3 + strlen(" -type f -follow -print || echo \"-[ERROR]\"") + 1];  //Creazione comando
                strcpy(command, "(test -f ");
                strcat(command, argv[i]);
                strcat(command, " || test -d ");
                strcat(command, argv[i]);
                strcat(command, ") && find ");
                strcat(command, argv[i]);
                strcat(command, " -type f -follow -print || echo \"-[ERROR]\"");
                //printf("%s\n",command);
                fp = popen(command, "r");  //avvia il comando e in fp prende l'output
                if (fp == NULL)            //Se il comando non va a buon fine
                {
                    value_return = err_args_A();
                } else {                                                                //Il comando va a buon fine
                    while (fgets(riga, sizeof(riga), fp) != NULL && errdir == FALSE) {  //Legge riga per riga e aggiunge alla lista
                        if (strcmp(riga, "-[ERROR]\n")) {
                            //memset( resolved_path, '\0', sizeof(resolved_path));
                            realpath(riga, resolved_path);                 //risalgo al percorso assoluto
                            resolved_path[strlen(resolved_path) - 1] = 0;  //tolgo l'ultimo carattere che manderebbe a capo
                            char str[12];
                            char path[PATH_MAX];
                            sprintf(str, "%d", *count);
                            strcpy(path, str);
                            strcat(path, "#");
                            strcat(path, resolved_path);
                            //printf("%s\n",path);
                            if (insertPathList(lista, path, 0)) {
                                (*count)++;
                                //printf("%s\n", resolved_path);
                            }

                        } else {            //Intercetta l'errore riguardante file o cartelle non esistenti
                            errdir = TRUE;  //Metto il flag errore file/directory sbagliati
                        }
                    }
                    pclose(fp);                                               //chiudo fp
                    if (errdir == TRUE) value_return = err_input_A(argv[i]);  //Mando l'errore per la directory
                }
            } else {
                flag = FALSE;  //Analisi argomento saltata, rimetto flag a false
            }
        }
        if (count == 0 && value_return == 0) value_return = err_args_A();  //counter is higher than zero, if not gives an error (value_return used to avoid double messages)
    }

    return value_return;
}

void initialize_processes(pid_t *p, int dim) {
    int i;
    for (i = 0; i < dim; i++) {
        p[i] = -1;
    }
}

void add_process_to_v(pid_t f, int *v) {
    int i = 0;
    while (v[i] != 0) {
        i++;
    }
    v[i] = f;
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

///src/Analyzer/Q.c
//Initialize frequence vector all to 0
void initialize_vector(int *v) {
    int i;
    for (i = 0; i < DIM_V; i++) {
        v[i] = 0;
    }
}

//Increase frequence of the global vector in the position val_ascii
void set_add(int *v, char c) {
    int val_ascii;
    val_ascii = ((int)c) - 32;  //casting char to int and difference 32 (in order to save space on the vector) //Se vogliamo togliere lo spazio basta fare -33
    v[val_ascii]++;
}

//get the chars from the .txt files from the begin (b) to the end (e)
void get_subset(FILE *fp, int *v, int b, int e) {
    int i;
    char c;
    fseek(fp, b, SEEK_SET);    //setting initial position of SEEK cursor
    for (i = b; i < e; i++) {  //P.S.: il primo carattere non è compreso, l'ultimo si

        if (feof(fp)) {
            printf("[!] Errore feof\n");
            //err_end_file();
            break;
        } else {
            fscanf(fp, "%c", &c);  //gets char
            //printf("%c", c);      //display what you asked the process to analyze (uncomment to use)
            if (c != '\n') {
                set_add(v, c);  //aggiunge al vettore delle frequenze il carattere c
            }
        }
    }
}

void get_frequencies(FILE *fp, int *freq, int part, int m) {  //Prima di commentarlo bene testiamo
    //int *freq = malloc(sizeof(int*) * DIM_V); //where frequencies will be stored
    initialize_vector(freq);
    int i = 0;
    int file_length = file_len(fp);
    int char_parts = file_length / m;
    int rest = file_length - (char_parts * m);
    while (i != part) {
        if (rest != 0) {
            rest--;
        }
        i++;
    }
    int begin = i * char_parts;
    int end = begin + char_parts;
    if (rest != 0) {
        end++;
    }
    get_subset(fp, freq, begin, end);

    //return &freq[0];
}

//display how meny times chars are in the text (display only visited chars)
void print_vector(int v[]) {
    int i;
    for (i = 0; i < DIM_V; i++) {
        if (v[i] != 0) {
            printf("\n%c è comparso %d volte", (i + 32), v[i]);
        }
    }
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

void createCsv(int *v, char *res, char *id) {
    int i;
    char str[12];
    for (i = 0; i < DIM_RESP; i++) {
        res[i] = '\0';
    }

    strcat(res, id);
    strcat(res, "#");
    sprintf(str, "%d", v[0]);
    strcat(res, str);
    for (i = 1; i < DIM_V; i++) {
        strcat(res, ",");
        sprintf(str, "%d", v[i]);
        strcat(res, str);
    }
    strcat(res, "#");
}

///src/C.c
char sumCsv(char *str1, char *str2) {
    char ret = FALSE;
    char *id1 = strtok(strdup(str1), "#");
    char *tmp1 = strtok(NULL, "#");

    char *id2 = strtok(strdup(str2), "#");
    char *tmp2 = strtok(NULL, "#");
    int v[DIM_RESP];
    initialize_vector(v);

    if (!strcmp(id1, id2)) {
        addCsvToArray(tmp1, v);
        addCsvToArray(tmp2, v);
        createCsv(v, str1, id1);
        ret = TRUE;
    }

    return ret;
}

char addCsvToArray(char *tmp, int *v) {
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

///src/R.c
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

void printStat_Cluster(char *char_count) {
    int v[DIM_V];
    int i;
    int lettereMin = 0;
    int lettereMai = 0;
    int spazi = 0;
    int numeri = 0;
    int punt = 0;
    int carSpec = 0;
    int tot = 0;
    parse_string(char_count, v);
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

    printf("STAMPA STATISTICHE PER CLUSTER\n\n");
    printf("Lettere minuscole:\t %d\t%.4g%%\n", lettereMin, (float)lettereMin / (float)tot * 100);
    printf("Lettere maiuscole:\t %d\t%.4g%%\n", lettereMai, (float)lettereMai / (float)tot * 100);
    printf("Spazi:\t\t\t %d\t%.4g%%\n", spazi, (float)spazi / (float)tot * 100);
    printf("Numeri:\t\t\t %d\t%.4g%%\n", numeri, (float)numeri / (float)tot * 100);
    printf("Segni di punteggiatura:\t %d\t%.4g%%\n", punt, (float)punt / (float)tot * 100);
    printf("Caratteri speciali:\t %d\t%.4g%%\n", carSpec, (float)carSpec / (float)tot * 100);
    printf("\n");
    printf("Caratteri totali: %d\n", tot);
}

void printInfoCluster() {
    int i;
    printf("Lettere minuscole:\t");
    for (i = 65; i <= 90; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Lettere maiuscole:\t");
    for (i = 33; i <= 58; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Spazi:\t ");
    printf("\n");
    printf("Numeri:\t\t\t");
    for (i = 16; i <= 25; i++) {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Segni di punteggiatura:\t");
    for (i = 1; i <= 31; i++) {
        if (i == 1 || i == 2 || (i >= 7 && i <= 9) || (i >= 12 && i <= 15) || i == 26 || i == 27 || i == 31) {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
    printf("Caratteri speciali:\t");
    for (i = 3; i <= 94; i++) {
        if ((i >= 3 && i <= 6) || i == 10 || i == 11 || (i >= 28 && i <= 30) || i == 32 || (i >= 59 && i <= 64) || (i >= 91 && i <= 94)) {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
}

void percAvanzamento(int n, int tot) {
    int hashtag = (int)((float)n * 10 / (float)tot);
    int i;
    printf("[");
    for (i = 0; i < 10; i++) {
        if (i < hashtag)
            printf("#");
        else
            printf(".");
    }
    printf("]\n");
}

///src/Analyzer/P.c
//return file length in terms of chars
int file_len(FILE *fp) {
    int len = 0;
    char c;
    while (!feof(fp)) {
        fscanf(fp, "%c", &c);
        len++;
        //printf("%d", len);
    }
    return len - 1;
}

m_process *splitter(FILE *fp, int m) {
    //This is for splitting the parts
    m_process *div = malloc(m);
    int file_length = file_len(fp);   //get the length of the file in terms of chars
    int int_div = (file_length / m);  //get the integer of the subdivision of the file
    //double double_div = ((double)file_length / (double)m); //get the double of the subdivision of the file
    int rest = file_length - (int_div * m);  //get the number of chars that are out of consideration
    int i;

    for (i = 0; i < m; i++) {
        if (i == 0) {
            div[i].begin = 0;
        } else {
            div[i].begin = div[i - 1].end;
        }
        div[i].end = div[i].begin + int_div;
        //printf("\nRest: %d", rest);
        if (rest != 0) {
            div[i].end += 1;
            rest--;
        }
        div[i].part = i;
    }

    return div;
}

//The next 3 functions are for conversion of (int) to (char*)
// inline function to swap two numbers
inline void swap(char *x, char *y) {
    char t = *x;
    *x = *y;
    *y = t;
}

// function to reverse buffer[i..j]
char *reverse(char *buffer, int i, int j) {
    while (i < j)
        swap(&buffer[i++], &buffer[j--]);

    return buffer;
}

// Iterative function to implement itoa() function in C
char *itoa(int value, char *buffer, int base) {
    // invalid input
    if (base < 2 || base > 32)
        return buffer;

    // consider absolute value of number
    int n = abs(value);

    int i = 0;
    while (n) {
        int r = n % base;

        if (r >= 10)
            buffer[i++] = 65 + (r - 10);
        else
            buffer[i++] = 48 + r;

        n = n / base;
    }

    // if number is 0
    if (i == 0) buffer[i++] = '0';

    // If base is 10 and value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) buffer[i++] = '-';

    buffer[i] = '\0';  // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

void arrayToCsv(int *v, char *res) {
    int i;
    char str[12];
    for (i = 0; i < DIM_RESP; i++) {
        res[i] = '\0';
    }
    sprintf(str, "%d", v[0]);
    strcat(res, str);
    for (i = 1; i < DIM_V; i++) {
        strcat(res, ",");
        sprintf(str, "%d", v[i]);
        strcat(res, str);
    }
}

//Error handlers
int err_pipe() {
    printf("Errore nella creazione della pipe\n");
    return ERR_PIPE;
}

int err_args_A() {
    printf("\nErrore nella sintassi del comando.\nUsa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    return ERR_ARGS_A;
}

int err_overflow() {
    printf("\nErrore overflow\nI risultati delle analisi sono troppo grandi\nProva con meno file o con file meno pesanti\n\n");
    return ERR_ARGS_A;
}

int err_input_A(char *file) {
    printf("\nErrore input\nFile/Directory non esistente: %s\n\n", file);
    return ERR_ARGS_A;
}

int err_args_C() {
    printf("\nErrore nella sintassi del comando.Usa:\n-nfiles <int> per indicare il numero di files (necessario)\n-setn <int> per settare n\n-setm <int> per settare m.\n\n");
    return ERR_ARGS_C;
}

int err_args_Q() {
    printf("[!] Errore nella sintassi del comando\nusa: ./Q inizio_analisi fine_analisi\n");
    return ERR_ARGS_Q;
}

int err_args_P() {
    printf("[!] Errore nella sintassi del comando\nusa: ./P m\n");
    return ERR_ARGS_P;
}

int err_file() {
    printf("Errore, nessun file inserito\n");
    return ERR_FILE;
}
//Error if 3 arguments are inserted //Refers to Q.c
int err_fork() {
    printf("Errore, fork non riuscito\n");
    return ERR_FORK;
}

int err_write() {
    printf("Errore, write non riuscita\n");
    return ERR_WRITE;
}
//Error if fopen has failed //Refers to Q.c
int err_file_open() {
    printf("[!] Errore nell'apertura del file\n");
    return ERR_FILE;
}

//Error if end point is over EOF //Refers to Q.c
int err_end_file() {
    printf("\n[!] Errore, sei andato oltre la fine del file\n");
    return ERR_FILE;
}

int err_fcntl() {
    printf("Errore, sblocco pipe non riuscito\n");
    return ERR_FCNTL;
}

int err_exec(int err) {
    printf("Errore nell'esecuzione di exec(%d) in: %d\n", err, getpid());
    return ERR_EXEC;
}

int err_m_not_valid() {
    printf("[!] Il valore di m non è valido, deve essere m > 0\n");
    return ERR_DATA;
}

int err_part_not_valid() {
    printf("[!] Il valore di part non è valido, deve essere < m\n");
    return ERR_DATA;
}
int err_process_open(pid_t p) {
    printf("[!] Errore, il processo %d è ancora aperto!\n", p);
    return ERR_OPEN_PROC;
}
int err_fifo() {
    printf("Errore nella creazione della pipe fifo\n");
    return ERR_FIFO;
}

int err_unlink() {
    printf("Errore nella eliminzazione della pipe fifo\n");
    return ERR_UNLINK;
}

int err_signal() {
    fprintf(stderr, "Errore, call signal non riuscita\n");
    return ERR_SIGNAL;
}

int err_close() {
    fprintf(stderr, "Errore nella chiusura del file\n");
    return ERR_CLOSE;
}

int err_args_R() {
    fprintf(stderr, "Errore nella sintassi del comando. Usa:\n-c: Stampa per cluster\n");
    return ERR_ARGS_R;
}

int err_enxio() {
    fprintf(stderr, "Errore, R e' stato avviato senza un processo A\n");
    return ERR_ENXIO;
}

int err_args_M() {
    printf("\nErrore nella sintassi del comando.\nUsa: /M nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /M A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    return ERR_ARGS_M;
}