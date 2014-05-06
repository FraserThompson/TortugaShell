/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

/* -----CROSS PLATFORM----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
char *readline() {
	// line can be up to 100 chars
	char *line = malloc(sizeof(char)* 100);
	printf(">");

	if (fgets(line, 100, stdin) == NULL) {
		return 0;
	}
	else {
		return line;
	}
}

/* 
* Main loop. Reads a line and parses it.
*/
int main(int argc, char *argv[]) {
	while (1){
		parse(readline());
	}
	return 0;
}
