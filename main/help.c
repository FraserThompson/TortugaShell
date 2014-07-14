/*
* help.c
*
*  Created on: 16/05/2014
*      Author: Fraser
* Goes through the ./commands directory printing the name of each file and the -h text
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include "help.h"
#include "process_mgmt.h"
#include "parser.h"

/* -----WINDOWS----
* Searches the ./commands directory and prints each filename, and then calls the file with -h to get the help.
*/
void print_help(void){
	char *path_commands = concat_string(PATH, "\\commands\\", NULL);
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t *sDir = convert_to_wchar(path_commands);
	wchar_t sPath[2048];
	int error;

	wsprintf(sPath, L"%s\\*.exe", sDir);

	// First display the hardcoded built in commands
	printf("\n\t----------BUILT IN COMMANDS----------\n\n");
	printf("cd\tChanges the current working directory.\n\tUsage: cd [directory]\n");
	printf("cwd\tPrints the current working directory.\n\tUsage: cwd\n");
	printf("help\tPrints this help message.\n\tUsage: help\n");

	// Then work on the binaries in the commands subdirectory
	printf("\n\t----------EXTERNAL COMMANDS----------\n\n");

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return;
	}

	do
	{
		{
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) 
			{
				continue;
			} else {
				error = create_process(convert_to_char(sPath), "-h", NULL, NULL);
				if (error != 0) {
					printf("PRINT_HELP: Could not open.\n");
					return;
				}
			}
		}
	} while (FindNextFile(hFind, &fdFile)); 
	printf("\n"); // makes it tidier
	FindClose(hFind); 
}