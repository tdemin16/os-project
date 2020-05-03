#!/bin/bash

help:
	cat ./README.md

build:
	if [ ! -d "./src/bin" ]; then \
	mkdir -p ./src/bin; fi
	gcc -c -std=gnu90 -o ./src/bin/lib ./src/lib/lib.c
	gcc -c -std=gnu90 -o ./src/bin/Q ./src/Analyzer/Q.c
	gcc -c -std=gnu90 -o ./src/bin/P ./src/Analyzer/P.c
	#gcc -c -std=gnu90 -o ./src/bin/C ./src/Analyser/C.c
	gcc -c -std=gnu90 -o ./src/bin/A ./src/Analyzer/A.c
	gcc -c -std=gnu90 -o ./src/bin/R ./src/R.c
	#gcc -c -std=gnu90 -o ./src/bin/M ./src/M.o

clean:
	rm ./src/bin/lib
	rm ./src/bin/Q
	rm ./src/bin/P
	#rm ./src/bin/C
	rm ./src/bin/A
	rm ./src/bin/R
	#rm ./src/bin/M
	rm -r ./src/bin

.PHONY: help build clean

./src/bin/lib: ./src/lib/lib.h ./src/lib/lib.c
	gcc -c -std=gnu90 -o ./src/bin/lib ./src/lib/lib.c

./src/bin/Q: ./src/bin/lib ./src/Analyzer/Q.c
	gcc -c -std=gnu90 -o ./src/bin/Q ./src/Analyzer/Q.c

./src/bin/P: ./src/bin/lib ./src/Analyzer/P.c
	gcc -c -std=gnu90 -o ./src/bin/P ./src/Analyzer/P.c

#./src/bin/C: ./src/bin/lib ./src/Analyzer/C.c
#	gcc -c -std=gnu90 -o ./src/bin/C ./src/Analyzer/C.c

./src/bin/A: ./src/bin/lib ./src/Analyzer/A.c
	gcc -c -std=gnu90 -o ./src/bin/A ./src/Analyzer/A.c

./src/bin/R: ./src/bin/lib ./src/R.c
	gcc -c -std=gnu90 -o ./src/bin/R ./src/R.c

#./src/bin/M.o: ./src/bin/lib.o ./src/M.c
#	gcc -c -std=gnu90 -o ./src/bin/M.o ./lib/M.c
