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
#include "../help.h"
#include "../../main/process_mgmt.h"
int debug_global = 0;

int main(int argc, char *argv[]){
	print_help();
	exit(EXIT_SUCCESS);
}

/* -----WINDOWS----
* Searches the ./commands directory and prints each filename, and then calls the file with -h to get the help.
*/
void print_help(void){
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t *sDir = L".\\commands";
	wchar_t sPath[2048];
	int error;

	wsprintf(sPath, L"%s\\*.exe", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return 0;
	}

	do
	{
		{
			wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) 
			{
				continue;
			} else {
				wprintf(L"%s: \t", fdFile.cFileName);
				if (strcmp(fdFile.cFileName, L"help.exe")){
					error = create_process(convert_to_char(sPath), "-h");
					if (error != 0) {
						printf("PRINT_HELP: Could not open.\n");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	} while (FindNextFile(hFind, &fdFile)); 

	FindClose(hFind); 
}