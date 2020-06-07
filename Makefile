#!/bin/bash

.PHONY: help build clean

help:
	@echo "\nmake build	[ primo comando ]"
	@echo "make clean	[ elimina il contenuto della cartella ./bin/ ]"
	@echo "make help	[ mostra questo testo. in futuro il contenuto di README.md ]\n"
	@echo "-----------------------------------------------------------------------------"
	cat ./README.txt

build:
	@if [ ! -d "./bin" ]; then \
	mkdir ./bin; fi
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/A.c -o ./bin/A
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/C.c -o ./bin/C
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/P.c -o ./bin/P
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/Q.c -o ./bin/Q
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/R.c -o ./bin/R
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/M.c -o ./bin/M

debug:
	@if [ ! -d "./bin" ]; then \
	mkdir ./bin; fi
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/A.c -o ./bin/A -D DEBUG
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/C.c -o ./bin/C -D DEBUG
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/P.c -o ./bin/P -D DEBUG
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/Analyzer/Q.c -o ./bin/Q -D DEBUG
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/R.c -o ./bin/R -D DEBUG
	@gcc -g -Werror -Wall -Wunused -std=gnu90 ./src/lib/lib.c ./src/M.c -o ./bin/M -D DEBUG

clean:
	@rm -rf ./bin