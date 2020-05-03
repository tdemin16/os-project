#!/bin/bash

.PHONY: help build clean

help:
	@echo "\nmake build	[ primo comando ]"
	@echo "make clean	[ elimina il contenuto della cartella ./bin/ ]"
	@echo "make help	[ mostra questo testo. in futuro il contenuto di README.md ]\n"
	@#cat ./README.md

build:
	@if [ ! -d "./bin" ]; then \
	mkdir -p ./bin; fi
	@gcc -std=gnu90 ./src/lib/lib.c ./src/lib/lib.c -o ./bin/lib 
	@gcc -std=gnu90 ./src/Analyzer/P.c ./src/lib/lib.c -o ./bin/P 
	@gcc -std=gnu90 ./src/Analyzer/C.c ./src/lib/lib.c -o ./bin/C
	@gcc -std=gnu90 ./src/Analyzer/Q.c ./src/lib/lib.c -o ./bin/Q
	@gcc -std=gnu90 ./src/Analyzer/A.c ./src/lib/lib.c -o ./bin/A 
	#@gcc -c -std=gnu90 ./src/R.c -o ./bin/R 
	#@#@gcc -c -std=gnu90 -o ./bin/M ./src/M.o

clean:
	@rm -rf ./bin