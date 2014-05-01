/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

/*Prints a prompt, interprets user input*/
char *readline() {
	wchar_t *line = malloc(sizeof(wchar_t)* 20);
	printf(">");

	if (fgets(line, 20, stdin) == NULL) {
	}
	else {
		return line;
	}
}

int main(int argc, char *argv[]) {
	while (1){
		parse(readline());
	}
	return 0;
}
