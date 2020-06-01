# Analyzer
## Descrizione
Il progetto si compone di 6 file eseguibili e di una libreria (da noi composta) contenete funzioni utili. Solo 3 però sono gli eseguibili che possono essere avviati dal terminale **M**, **R** ed **A**.

## A
Il compito del processo **A** è di eseguire l'analisi dei percorsi che gli vengono forniti in input. Per eseguire questa procedura vengono generati:
* Un processo **C**
* *n* processi **P** (default: 3)
* *m* processi **Q** (default: 4)
Il processo A può essere avviato con alcuni argomenti in input che possono essere, percorsi di file e/o cartelle, -setn x, -setm y (es. ./A ../src ../test/test1.txt -setn 2 -setm 6, questo avvierà l'analisi sulla cartella src e sul file test1.txt con n=2 e m=6).

Se il processo non dovesse essere avviato con argomenti in input oppure si vuole eseguire alcune operazioni ad analisi terminata, sono a disposizione diversi comandi:
* add path1 path2 ...: permette di aggiungere alla lista dei file analizzabili i percorsi inseriti
* remove path1 path2 ...: permette di rimuovere dalla lista dei file analizzabili i percorsi inseriti
* analyze: analizza i percorsi inseriti non ancora analizzati
* reanalyze: esegue l'analisi su tutti i percorsi, anche quelli già analizzati
* reset: rimuove tutti i file inseriti
* set n m: cambia il numero di processi P e Q generati (questa operazione è garantita anche durante l'analisi)
* setn n
* setm m

Una volta avviato A, viene eseguito un fork e generato un processo C. Questo avrà il compito di gestire la generazione dei processi P e il cambiamento del numero di P e Q. C poi distribuirà ai processi P i file in maniera equa in modo da dividere il carico su n processi. I processi P genereranno a loro volta m processi Q e, inoltre, si occuperà del riavvio di questi quando il valore m viene modificato. I processi Q, infine, spartiscono tra di loro ogni file riducendo distribuendo ulteriormente il carico.

### Scelte implementative
Dato che si trattano di processi generati in line gerarchica, è stato possibile l'uso di pipe anonime per la comunicazione tra i vari processi. Inoltre per garantire una maggiore parallelizzazione alle pipe è stato assegnato il flag **O_NONBLOCK** per evitare blocchi in lettura e scrittura. //Tb be continued...
