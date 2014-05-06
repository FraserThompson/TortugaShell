/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>


/* -------WINDOWS------
* Returns the CWD in windows
*/
void *getCWD_win(char *cwd){

	if (!GetCurrentDirectory(1024, cwd)) {
		printf("Error getting current directory! %s\n", GetLastError());
	}
}

/* -------UNIX------
* Return the CWD in Unix/Linux
*/
char *getCWD_unix(){
}

/* -----CROSS PLATFORM----
*Returns the path to the cwd
*/
int main(int argc, char *argv[]){
	char cwd[1024] = "";
	getCWD_win(cwd);
	printf("%ws\n", cwd);
	return EXIT_SUCCESS;
}
