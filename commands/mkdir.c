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
	int i = 0;
	//Help message, printed by default if no arguments
	while (argv[i]){
		if (argc == 1 || ((wcscmp(argv[i], L"-help") == 0) || (wcscmp(argv[i], L"-h") == 0))) {
			wprintf(L"rmdir\tCreates a directory.\n\tUsage: mkdir [dir]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}
	
	if (!CreateDirectory(argv[1], NULL)) {
		wprintf(L"\nCouldn't create %ls directory.\n", argv[1]);
	}
	else {
		wprintf(L"\n%ls directory successfully created.\n", argv[1]);
	}
}