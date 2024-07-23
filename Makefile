all:
	gcc -std=c99 -Os -Wall -pedantic wordle.c words.c -o wordle

exe:
	cl /nologo /DDEBUG /D_CRT_SECURE_NO_WARNINGS /Os /std:c11 /Wall /wd4668 /wd4820 /wd4061 /wd5045 wordle.c words.c 
