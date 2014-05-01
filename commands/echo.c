/*
 * echo.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

/*prints the string*/
int main(int argc, char *argv[]){
	if (argc > 0){
		printf("%s\n", argv[1]);
		return 1;
	}
	printf("Nothing to say!");
	return 0;
}
