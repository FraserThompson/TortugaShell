/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
int debug_global = 1;

/* -----CROSS PLATFORM----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static char *readline(void) {
	// line can be up to 300 chars
	char *line = malloc(sizeof(char)* 300);
	printf(">");

	if (fgets(line, 300, stdin) == NULL) {
		fprintf(stderr, "READLINE: Error reading line!");
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
	
	if (argc > 1){
		if ((strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "-debug") == 0)) {
			debug_global = 1;
		}
	}

	while (1){
		parse(readline());
	}
	return EXIT_SUCCESS;
}
