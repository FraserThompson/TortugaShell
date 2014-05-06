/*
 * echo.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

/*Prints the string*/
int main(int argc, char *argv[]){

	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Prints user input to the terminal. If no input prints location it's running from.\n");
		return EXIT_SUCCESS;
	}

	printf("%s\n", argv[0]);
	return EXIT_SUCCESS;
}
