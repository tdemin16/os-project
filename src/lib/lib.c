#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "lib.h"

array *createPathList(int size)
{
    array *st = (array *)malloc(sizeof(array));
    st->size = size;
    st->pathList = malloc(sizeof(char **) * size);
    st->count = 0;
    int i;
    for (i = 0; i < size; i++)
    {
        st->pathList[i] = (char *)malloc(sizeof(char *) * (PATH_MAX+1));
    }
    return st;
}

char insertPathList(array *tmp, char *c)
{
    int i;
    char present = FALSE;
    char ret = FALSE;

    for (i = 0; i<tmp->count; i++){
        if (!strcmp(tmp->pathList[i], c)){
            present = TRUE;
        }
    }
    if (present == FALSE){
        //printf("Provo a inserire %s, size: %d, count:%d\n", c, tmp->size, tmp->count);
        if (tmp->count == tmp->size)
        {
            //printf("Raddoppio la size\n");
            tmp->size *= 2;
            tmp->pathList = (char **)realloc(tmp->pathList, sizeof(char **) * tmp->size);
            for (i = tmp->count; i < tmp->size; i++)
            {
                tmp->pathList[i] = (char *)malloc(sizeof(char *) * (PATH_MAX + 1));
            }
        }
        //printf("Stringa inserita\n");
            strcpy(tmp->pathList[tmp->count], c);
            tmp->count++;
            ret = TRUE;
    } else { ret = FALSE;}
    return ret;
}

void printPathList(array *tmp)
{
    int i;
    for (i = 0; i < tmp->count; i++)
    {
        printf("%d: %s\n", i, tmp->pathList[i]);
    }
}

int dimPathList(array *tmp){
    return tmp->count;
}

void freePathList(array *tmp)
{
    int i;
    for (i = 0; i < tmp->size; i++)
    {
        free(tmp->pathList[i]);
    }
    free(tmp->pathList);
    free(tmp);
}

int unlock_pipes(int *fd, int size)
{
    int i;
    int ret = 0;
    for (i = 0; i < size && ret == 0; i += 2)
    {
        if (fcntl(fd[i], F_SETFL, O_NONBLOCK))
        {
            ret = -1;
        }
    }
    return ret;
}

void close_pipes(int *fd, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        close(fd[i]);
    }
}

// /src/Analyzer/A.c
int parse_string(char *string, int v[DIM_V])
{
    int i = 0;
    int max = 0;
    char *token = strtok(string, ",");
    while (token != NULL)
    { //loop through token
        if (i < DIM_V)
        {
            v[i] = atoi(token);
            if (v[i] > max)
                max = v[i];
            i++;
            token = strtok(NULL, ",");
        }
        else
        {
            printf("Errore nella creazione della stringa contatore");
        }
    }
    return max;
}

///src/Analyzer/Q.c
//Initialize frequence vector all to 0
void initialize_vector(int* v)
{
    int i;
    for (i = 0; i < DIM_V; i++)
    {
        v[i] = 0;
    }
}

//Increase frequence of the global vector in the position val_ascii
void set_add(int* v, char c)
{
    int val_ascii;
    val_ascii = ((int)c) - 32; //casting char to int and difference 32 (in order to save space on the vector) //Se vogliamo togliere lo spazio basta fare -33
    v[val_ascii]++;
}

int* get_frequencies(FILE *fp, int part, int m) //Prima di commentarlo bene testiamo
{
    int *v = malloc(sizeof(int*) * DIM_V); //where frequencies will be stored
    initialize_vector(v);
    int i = 0;
    int file_length = file_len(fp);
    int char_parts = file_length / m;
    int rest = file_length - (char_parts * m);
    while (i != part)
    {
        if (rest != 0)
        {
            rest--;
        }
        i++;
    }
    int begin = i * char_parts;
    int end = begin + char_parts;
    if (rest != 0)
    {
        end++;
    }

    get_subset(fp, v, begin, end);

    return v;
}

//get the chars from the .txt files from the begin (b) to the end (e)
void get_subset(FILE *fp, int* v, int b, int e)
{
    int i;
    char c;
    fseek(fp, b, SEEK_SET); //setting initial position of SEEK cursor
    for (i = b; i < e; i++) //P.S.: il primo carattere non è compreso, l'ultimo si
    {
        if (feof(fp))
        {
            err_end_file();
            break;
        }
        else
        {
            fscanf(fp, "%c", &c); //gets char
            //printf("%c", c);      //display what you asked the process to analyze (uncomment to use)
            if (c != '\n')
            {
                set_add(v, c); //aggiunge al vettore delle frequenze il carattere c
            }
        }
    }
}

//display how meny times chars are in the text (display only visited chars)
void print_vector(int v[])
{
    int i;
    for (i = 0; i < DIM_V; i++)
    {
        if (v[i] != 0)
        {
            printf("\n%c è comparso %d volte", (i + 32), v[i]);
        }
    }
}


int lenghtCsv(int v[DIM_V]){
    int i;
    int dim = 0;
    for (i = 0;i<DIM_V;i++){
        dim += countDigit(v[i]);  
    }
    dim +=95;
    return dim;
}


int countDigit(int n) 
{ 
    int count = 0;
    if (n == 0){
        count++;
    } else {
    while (n != 0) { 
        n = n / 10; 
        ++count; 
    } }
    return count; 
} 


///src/R.c
void printStat(char *char_count)
{
    int v[DIM_V];
    int i, k;
    int column = 9;
    int max = parse_string(char_count, v);
    printf("STAMPA STATISTICHE GENERALI\n\n");
    if (max <= 9999999)
    {
        for (i = 0; i < DIM_V; i += column)
        {
            for (k = i; k < i + column && k < DIM_V; k++)
            {
                printf("%c\t", k + 32);
            }
            printf("\n");
            for (k = i; k < i + column && k < DIM_V; k++)
            {
                printf("%d\t", v[k]);
            }
            printf("\n\n");
        }
    }
    else
    {
        column = 5;
        for (i = 0; i < DIM_V; i += column)
        {
            for (k = i; k < i + column && k < DIM_V; k++)
            {
                printf("%c\t\t", k + 32);
            }
            printf("\n");
            for (k = i; k < i + column && k < DIM_V; k++)
            {
                printf("%d\t", v[k]);
                if (v[k] < 9999999)
                    printf("\t");
            }
            printf("\n\n");
        }
    }
}

void printStat_Cluster(char *char_count)
{
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
    for (i = 0; i < DIM_V; i++)
    {
        if (i == 0){
            spazi += v[i];
            tot+=v[i];
            }
        if (i == 1 || i == 2 || (i >= 7 && i <= 9) || (i >= 12 && i <= 15) || i == 26 || i == 27 || i == 31){
            punt += v[i];
            tot+=v[i];
            }
        if ((i >= 3 && i <= 6) || i == 10 || i == 11 || (i >= 28 && i <= 30) || i == 32 || (i >= 59 && i <= 64) || (i >= 91 && i <= 94)){
            carSpec += v[i];
            tot+=v[i];
            }
        if (i >= 16 && i <= 25){
            numeri += v[i];
            tot+=v[i];
            }
        if (i >= 33 && i <= 58){
            lettereMai += v[i];
            tot+=v[i];
            }
        if (i >= 65 && i <= 90){
            lettereMin += v[i];
            tot+=v[i];
            }
    }
    printf("%d",tot);
    printf("STAMPA STATISTICHE PER CLUSTER\n\n");
    printf("Lettere minuscole:\t %d\t%.4g%%\n", lettereMin, (float)lettereMin/(float)tot*100);
    printf("Lettere maiuscole:\t %d\t%.4g%%\n", lettereMai, (float)lettereMai/(float)tot*100);
    printf("Spazi:\t\t\t %d\t%.4g%%\n", spazi, (float)spazi/(float)tot*100);
    printf("Numeri:\t\t\t %d\t%.4g%%\n", numeri, (float)numeri/(float)tot*100);
    printf("Segni di punteggiatura:\t %d\t%.4g%%\n", punt, (float)punt/(float)tot*100);
    printf("Caratteri speciali:\t %d\t%.4g%%\n", carSpec, (float)carSpec/(float)tot*100);
    printf("\n");
}

void printInfoCluster()
{
    int i;
    printf("Lettere minuscole:\t");
    for (i = 65; i <= 90; i++)
    {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Lettere maiuscole:\t");
    for (i = 33; i <= 58; i++)
    {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Spazi:\t ");
    printf("\n");
    printf("Numeri:\t\t\t");
    for (i = 16; i <= 25; i++)
    {
        printf("%c ", i + 32);
    }
    printf("\n");
    printf("Segni di punteggiatura:\t");
    for (i = 1; i <= 31; i++)
    {
        if (i == 1 || i == 2 || (i >= 7 && i <= 9) || (i >= 12 && i <= 15) || i == 26 || i == 27 || i == 31)
        {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
    printf("Caratteri speciali:\t");
    for (i = 3; i <= 94; i++)
    {
        if ((i >= 3 && i <= 6) || i == 10 || i == 11 || (i >= 28 && i <= 30) || i == 32 || (i >= 59 && i <= 64) || (i >= 91 && i <= 94))
        {
            printf("%c ", i + 32);
        }
    }
    printf("\n");
}

///src/Analyzer/P.c
//return file length in terms of chars
int file_len(FILE *fp)
{
    int len = 0;
    char c;
    while (!feof(fp))
    {
        fscanf(fp, "%c", &c);
        len++;
        //printf("%d", len);
    }
    return len - 1;
}

m_process *splitter(FILE *fp, int m)
{
    //This is for splitting the parts
    m_process *div = malloc(m);
    int file_length = file_len(fp);  //get the length of the file in terms of chars
    int int_div = (file_length / m); //get the integer of the subdivision of the file
    //double double_div = ((double)file_length / (double)m); //get the double of the subdivision of the file
    int rest = file_length - (int_div * m); //get the number of chars that are out of consideration
    int i;

    for (i = 0; i < m; i++)
    {
        if (i == 0)
        {

            div[i].begin = 0;
        }
        else
        {
            div[i].begin = div[i - 1].end;
        }
        div[i].end = div[i].begin + int_div;
        //printf("\nRest: %d", rest);
        if (rest != 0)
        {
            div[i].end += 1;
            rest--;
        }
        div[i].part = i;
    }

    return div;
}

//The next 3 functions are for conversion of (int) to (char*)
// inline function to swap two numbers
inline void swap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

// function to reverse buffer[i..j]
char *reverse(char *buffer, int i, int j)
{
    while (i < j)
        swap(&buffer[i++], &buffer[j--]);

    return buffer;
}

// Iterative function to implement itoa() function in C
char *itoa(int value, char *buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32)
        return buffer;

    // consider absolute value of number
    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
            buffer[i++] = 65 + (r - 10);
        else
            buffer[i++] = 48 + r;

        n = n / base;
    }

    // if number is 0
    if (i == 0)
        buffer[i++] = '0';

    // If base is 10 and value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10)
        buffer[i++] = '-';

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

//Error handlers
int err_pipe()
{
    printf("Errore nella creazione della pipe\n");
    return ERR_PIPE;
}

int err_args_A()
{
    printf("\nErrore nella sintassi del comando.\nUsa: /A nomeFile nomeCartella\nPuoi usare -setn e -setm per cambiare n e m\nes: /A A.c ../Analyzer/ -setn 3 -setm 4\n\n");
    return ERR_ARGS_A;
}

int err_input_A(char *file)
{
    printf("\nErrore input\nFile/Directory non esistente: %s\n\n", file);
    return ERR_ARGS_A;
}

int err_args_C()
{
    printf("\nErrore nella sintassi del comando.Usa:\n-nfiles <int> per indicare il numero di files (necessario)\n-setn <int> per settare n\n-setm <int> per settare m.\n\n");
    return ERR_ARGS_C;
}

int err_args_Q()
{
    printf("[!] Errore nella sintassi del comando\nusa: ./Q inizio_analisi fine_analisi\n");
    return ERR_ARGS_Q;
}

int err_args_P()
{
    printf("[!] Errore nella sintassi del comando\nusa: ./P m\n");
    return ERR_ARGS_Q;
}

int err_file()
{
    printf("Errore, nessun file inserito\n");
    return ERR_FILE;
}
//Error if 3 arguments are inserted //Refers to Q.c
int err_fork()
{
    printf("Errore, fork non riuscito\n");
    return ERR_FORK;
}

int err_write()
{
    printf("Errore, write non riuscita\n");
    return ERR_WRITE;
}
//Error if fopen has failed //Refers to Q.c
int err_file_open()
{
    printf("[!] Errore nell'apertura del file\n");
    return ERR_FILE;
}

//Error if end point is over EOF //Refers to Q.c
int err_end_file()
{
    printf("\n[!] Errore, sei andato oltre la fine del file\n");
    return ERR_FILE;
}

int err_fcntl()
{
    printf("Errore, sblocco pipe non riuscito\n");
    return ERR_FCNTL;
}

int err_exec(int err)
{
    printf("Errore nell'esecuzione di exec(%d) in: %d\n", err, getpid());
    return ERR_EXEC;
}

int err_m_not_valid()
{
    printf("[!] Il valore di m non è valido, deve essere m > 0\n");
    return ERR_DATA;
}

int err_part_not_valid() {
    printf("[!] Il valore di part non è valido, deve essere < m\n");
    return ERR_DATA;
}