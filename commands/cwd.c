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
#include <Windows.h>

#ifdef WINDOWS
#elif LINUX
#elif OSX
#endif

/* -------WINDOWS------
* Returns the CWD in windows
*/
void *getCWD_win(char *cwd){

	if (!GetCurrentDirectory(1024, cwd)) {
		printf("Error getting current directory! %s\n", GetLastError());
	}
	return 0;
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

	char cwd[1024] = "";
	getCWD_win(cwd);
	printf("%ws\n", cwd);
	return EXIT_SUCCESS;
}
