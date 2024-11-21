all: wordle

wordle: wordle.c words.c words.h Makefile
	gcc -std=c17 -Os -Wall -pedantic wordle.c words.c -o wordle

exe: Makefile words.c wordle.c
	cl /nologo /D_CRT_SECURE_NO_WARNINGS /std:c99 /Os /Wall /wd4668 /wd4820 /wd4061 /wd5045 wordle.c words.c
