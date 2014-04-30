/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

/*Prints a prompt, interprets user input*/
char *readline() {
	char line[20];
	char in[20];

	*in = fgets(line, 20, stdin);
	if (in == NULL) {

	}
	else {
		return line;
	}

}

int main(int argc, char *argv[]) {
	while (1){
		char *line = readline();
		parse(line);
	}
	return 0;
}
