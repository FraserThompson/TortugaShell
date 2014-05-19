/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 300
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "cwd.h"
int debug_global = 0;
char *PATH;

/* -----CROSS PLATFORM----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static char *readline(void) {
	char *line = malloc(sizeof(char)* MAX_LINE);
	printf("%s>", getCWD());

	if (fgets(line, MAX_LINE, stdin) == NULL) {
		fprintf(stderr, "READLINE: Error reading line!\n");
		exit(EXIT_FAILURE);
	}
	else {
		return line;
	}
}

/* 
* Main loop. Reads a line and parses it.
*/
int main(int argc, char *argv[]) {
	int i = 0;
	PATH = getCWD();

	while (argv[i]){
		if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	while (1){
		parse(readline());
	}
	return EXIT_SUCCESS;
}
