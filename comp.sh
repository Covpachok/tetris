#!/bin/sh

gcc -Wall -g -c tetris.c
gcc -Wall -g -c main.c
gcc -Wall -g -lncurses main.o tetris.o -o tetris
