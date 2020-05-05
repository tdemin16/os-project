#!/bin/bash

.PHONY: help build clean

help:
	@echo "\nmake build	[ primo comando ]"
	@echo "make clean	[ elimina il contenuto della cartella ./bin/ ]"
	@echo "make help	[ mostra questo testo. in futuro il contenuto di README.md ]\n"
	@#cat ./README.md

build:
	@sudo chmod -R 777 ./
	@if [ ! -d "./bin" ]; then \
	mkdir ./bin; fi
	@gcc -c ./src/lib/lib.c -std=gnu90 -o ./bin/lib
	@gcc -c -std=gnu90 -o ./bin/Q ./src/Analyzer/Q.c
	@gcc -c -std=gnu90 -o ./bin/P ./src/Analyzer/P.c
	@gcc -c -std=gnu90 -o ./bin/C ./src/Analyzer/C.c
	@gcc -c -std=gnu90 -o ./bin/A ./src/Analyzer/A.c
	@gcc -c -std=gnu90 -o ./bin/R ./src/R.c
	@#@gcc -c -std=gnu90 -o ./bin/M ./src/M.o

clean:
	@rm -rf ./bin
