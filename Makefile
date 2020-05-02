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
	@gcc -c -std=gnu90 -o ./bin/lib ./src/lib/lib.c
	@gcc -c -std=gnu90 -o ./bin/Q ./src/Analyzer/Q.c
	@gcc -c -std=gnu90 -o ./bin/P ./src/Analyzer/P.c
	@#@gcc -c -std=gnu90 -o ./bin/C ./src/Analyser/C.c
	@gcc -c -std=gnu90 -o ./bin/A ./src/Analyzer/A.c
	@gcc -c -std=gnu90 -o ./bin/R ./src/R.c
	@#@gcc -c -std=gnu90 -o ./bin/M ./src/M.o

clean:
	@rm ./bin/lib
	@rm ./bin/Q
	@rm ./bin/P
	@#@rm ./bin/C
	@rm ./bin/A
	@rm ./bin/R
	@#@rm ./bin/M


