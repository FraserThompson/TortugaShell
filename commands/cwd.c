/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*
*  Returns the path to the current working directory.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32 || _WIN64
	#include <Windows.h>
	#include <wchar.h>
#endif

/* -------WINDOWS------
* Returns the CWD in windows
*/
void *getCWD_win(){
	wchar_t cwd[1024];
	if (!GetCurrentDirectory(1024, cwd)) {
		printf("Error getting current directory! %s\n", GetLastError());
	}
	return cwd;
}

/* -------UNIX------
* Return the CWD in Unix/Linux
*/
char *getCWD_unix(){
	return NULL;
}

/* -----CROSS PLATFORM----
*Returns the path to the cwd
*/
int main(int argc, char *argv[]){
	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Displays the current working directory.\n");
		return EXIT_SUCCESS;
	}

	printf("%ws\n", getCWD_win());
	return EXIT_SUCCESS;
}
