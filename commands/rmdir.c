/*
* rmdir.c
*
* Created: 07/08/2014
* Author: James Buchanan
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* Remove a directory */

int wmain(int argc, wchar_t *argv[]){

	wchar_t *path = argv[1];
	if (RemoveDirectory(path) != 0) {
		printf("\nCouldn't remove %S directory.\n", path);
	}
	else {
		printf("\n%S directory removed successfully.\n", path);
	}
}