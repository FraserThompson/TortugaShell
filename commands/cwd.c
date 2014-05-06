/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>

/*Returns the path to the cwd*/
int main(int argc, char *argv[]){
	char cwd[1024] = "";

	if (!GetCurrentDirectory(1024, cwd)) {
		printf("Error getting current directory! %s\n", GetLastError());
	}

	printf("%ws\n", cwd);
	return 0;
}
