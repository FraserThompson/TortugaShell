/*
* mkdir.c
*
* Created: 29/07/2014
* Author: James Buchanan
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* Create new directory */

int wmain(int argc, wchar_t *argv[]){

	wchar_t *path = argv[1];
	if (!CreateDirectory(path, NULL)) {
		wprintf(L"\nCouldn't create %S directory.\n", path);
	}
	else {
		wprintf(L"\n%S directory successfully created.\n", path);
	}
}