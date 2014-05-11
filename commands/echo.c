/*
 * echo.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Prints the string*/
int main(int argc, char *argv[]){
	int i = 0;

	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Prints user input to the terminal. If no input prints location it's running from.\n");
		return EXIT_SUCCESS;
	}

	while (argv[i]){
		printf("%s ", argv[i++]);
	}
	printf("\n");
	return EXIT_SUCCESS;
}
