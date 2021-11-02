#!/bin/sh

gcc -Wall -g -c $1 tetris.c main.c etc.c
gcc -Wall -g -lncurses main.o tetris.o etc.o -o tetris
