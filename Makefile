#!/bin/bash

.PHONY: help build clean

help:
	@echo "\nmake build	[ primo comando ]"
	@echo "make clean	[ elimina il contenuto della cartella ./bin/ ]"
	@echo "make help	[ mostra questo testo. in futuro il contenuto di README.md ]\n"
	@echo "-----------------------------------------------------------------------------"
	cat ./README.md 

build:
	@if [ ! -d "./bin" ]; then \
	mkdir ./bin; fi
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/Analyzer/A.c -o ./bin/A
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/Analyzer/C.c -o ./bin/C
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/Analyzer/P.c -o ./bin/P
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/Analyzer/Q.c -o ./bin/Q
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/R.c -o ./bin/R
	@gcc -g -Werror -Wall -std=gnu90 ./src/lib/lib.c ./src/M.c -o ./bin/M

clean:
	@rm -rf ./bin
