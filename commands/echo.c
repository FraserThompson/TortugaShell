/*
 * echo.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 *
 *  Echo's the input args.
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* -----CROSS PLATFORM----
* Prints a string to stdout.
*/
int wmain(int argc, wchar_t *argv[]){
	int i = 0;

	//Help message, printed by default if no arguments
	while (argv[i]){
		if (argc == 1 || ((wcscmp(argv[i], "-help") == 0) || (wcscmp(argv[i], "-h") == 0))) {
			wprintf(L"echo\tPrints the user's message to the commandline.\n\tUsage: echo [message to print]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	i = 1;
	while (argv[i]){
		wprintf(L"%s ", argv[i++]);
	}
	wprintf(L"\n");
	return EXIT_SUCCESS;
}
