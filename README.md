# LabSO1-AA_2019_2020--200569_192435_193992_201487

E-Mail di Gruppo: thomas.demin@studenti.unitn.it

* Thomas De Min - 200569 - thomas.demin@studenti.unitn.it
* Benedetta Scattolin - 192435 - benedetta.scattolin@studenti.unitn.it
* Luigi Pusiol - 193992 - luigi.pusiol@studenti.unitn.it
* Andrea Bonomi - 201487 - andrea.bonomi-2@studenti.unitn.it

## Descrizione Progetto
Il programma e` stato sviluppato suddividendo i vari processi fra i componenti del gruppo.
### Q
> "Ogni processo Q analizza uno spezzone di ogni file di input a cui accede. "n" ed "m" hanno come valore
di default rispettivamente 3 e 4 ma possono essere passati come argomenti."

Q apre il file e riceve il contenuto in ASCII. In seguito passa al processo superiore (P) i vari dati raccolti e provvede a chiudere il file.

### P
> "Si occupa di gestire ciascuno un sottoinsieme degli input (partizionato in "n" sottogruppi) creando a sua volta "m" figli."

P si occupa di creare m processi figli (Q) e passare al processo padre (C) i dati raccolti.

### C
> "Un processo C ("Counter") principale tiene traccia dei conteggi (ricevendo quindi i dati) i quali devono
essere eseguiti da processi separati da esso generati."

C provvede a generare n-P processi ed a passare i dati ad A

### A
> "Un processo/sottosistema "A" ("Analyzer") per calcolare le statistiche (che coincide o comprende
l'albero descritto con C, P... e Q...)."

A si occupa della parte di calcolo delle statistiche e di passare i file/la cartella da analizzare ai processi figli

### R
> "Un processo/sottosistema "R" ("Report") per recuperare le informazioni."

R ha il solo ed unico compito di stampare le varie statistiche

### M
> "Un processo/sottosistema "M" ("Main") principale per la gestione generale: ha solo funzione di
"gateway" e interfaccia-utente. DEVE richiamare gli altri due processi/sottosistemi di seguito
indicati (che devono essere utilizzabili anche autonomamente direttamente da cli)."

M e` il processo che si occupa di gestire l'avvio di A e di R.

## Utilizzo
Da M: (Avviamo prima ./bin/M o lo avviamo con comandi???)
* Per analizzare una cartella di directory *dir* basta digitare da terminale "./bin/M *dir*";
* Per analizzare un file in posizione *dir* basta digitare da terminale "./bin/M *dir*";
* Per analizzare piu' file in posizione *dir_1*, *dir_2*, ..., *dir_n* e` sufficiente digitare "./bin/M *dir_1* *dir_2* *...* *dir_n*";
* Per settare m ed n basta aggiungere il flag *-setn* o *-setm* a seconda del valore da cambiare, per esempio: "./bin/M *dir* -setn *5* -setm *6*";
* Per chiudere il programma è sufficiente scrivere "close";
* Per visualizzare l'help digitare "help";
* Per visualizzare le info digitare "info";
* Per visualizzare le clustered info digitare "info -c"

Da A:
* Per analizzare una cartella di directory *dir* basta digitare da terminale "./bin/A *dir*";
* Per analizzare un file in posizione *dir* basta digitare da terminale "./bin/A *dir*";
* Per analizzare piu' file in posizione *dir_1*, *dir_2*, ..., *dir_n* e` sufficiente digitare "./bin/A *dir_1* *dir_2* *...* *dir_n*";
* Per settare m ed n basta aggiungere il flag *-setn* o *-setm* a seconda del valore da cambiare, per esempio: "./bin/A *dir* -setn *5* -setm *6*";

Da R:
* //TODO
