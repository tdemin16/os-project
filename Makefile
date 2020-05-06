#!/bin/bash

.PHONY: help build clean

help:
	@echo "\nmake build	[ primo comando ]"
	@echo "make clean	[ elimina il contenuto della cartella ./bin/ ]"
	@echo "make help	[ mostra questo testo. in futuro il contenuto di README.md ]\n"
	@echo "-----------------------------------------------------------------------------\n"
	@cat ./README.md

build:
	@sudo chmod -R 777 ./
	@if [ ! -d "./bin" ]; then \
	mkdir ./bin; fi
	@gcc -std=gnu90 ./src/lib/lib.c ./src/Analyzer/A.c -o ./bin/A
	@gcc -std=gnu90 ./src/lib/lib.c ./src/Analyzer/C.c -o ./bin/C
	@gcc -std=gnu90 ./src/lib/lib.c ./src/Analyzer/P.c -o ./bin/P
	@gcc -std=gnu90 ./src/lib/lib.c ./src/Analyzer/Q.c -o ./bin/Q
	@gcc -std=gnu90 ./src/lib/lib.c ./src/R.c -o ./bin/R
	@#@gcc -std=gnu90 ./src/lib/lib.c ./src/M.c -o ./bin/M

clean:
	@rm -rf ./bin
