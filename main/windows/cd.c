/*
* cd.c
*
*  Created on: 17/05/2014
*      Author: Fraser
*
*  Changes the current working directory.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "../cd.h"

/* -----CROSS-PLATFORM----
* Changes directory.
*/
void cd(char *dir) {
	int i = 0;

	if (chdir(dir) != 0) {
		printf("CD: Error! Malformed path maybe?\n");
		return EXIT_FAILURE;
	}
}

