/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*
*  Returns the path to the current working directory in Windows.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <wchar.h>
#include "../cwd.h"

/* -------WINDOWS------
* Returns the CWD in windows
*/
void *getCWD(){
	wchar_t cwd[1024];
	if (!GetCurrentDirectory(1024, cwd)) {
		printf("CWD: Error getting current directory! %s\n", GetLastError());
	}
	return cwd;
}

/* -----CROSS PLATFORM----
*Returns the path to the cwd
*/
int main(int argc, char *argv[]){
	int i = 0;

	//Help message, printed by default if no arguments
	while (argv[i]){
		printf("%s\n", argv[i]);
		if ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0)) {
			printf("CWD: Prints the path of the current working directory. Usage: cwd\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	printf("%ws\n", getCWD());
	return EXIT_SUCCESS;
}
