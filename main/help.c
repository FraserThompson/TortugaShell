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
#include "myStrings.h"
#include "process_mgmt.h"
#include "shell.h"

/* -----WINDOWS----
* Searches the ./commands directory and prints each filename, and then calls the file with -h to get the help.
*/
wchar_t *print_help(void){
	wchar_t *sDir = concat_string(PATH, L"\\commands\\", NULL);
	wchar_t sPath[2048];
	wchar_t *result;
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	command_line line = { NULL, NULL, NULL, NULL, 1 };
	int error;
	int debug_old = debug_global;
	debug_global = 0;

	wsprintf(sPath, L"%s\\*.exe", sDir);

	// First add the hardcoded built in commands
	wchar_t *built_in = L"\n\t----------BUILT IN COMMANDS----------\n\ncd\tChanges the current working directory.\n\tUsage: cd [directory]\ncwd\tPrints the current working directory.\n\tUsage: cwd\nhelp\tPrints this help message.\n\tUsage: help\n";

	// Then work on the binaries in the commands subdirectory
	wchar_t *externcomm = L"\n\t----------EXTERNAL COMMANDS----------\n\n";

	result = concat_string(built_in, externcomm, NULL);
	wprintf(L"%s\n", result);

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
				line.command = sPath;
				line.params = L"-h";
				error = create_child(line);
				if (error != 0) {
					printf("PRINT_HELP: Could not open.\n");
					return;
				}
			}
		}
	} while (FindNextFile(hFind, &fdFile)); 
	wprintf(L"\n"); // makes it tidier
	FindClose(hFind); 
	debug_global = debug_old;
	return result;
}