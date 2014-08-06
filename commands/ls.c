/*
* ls.c
*
* Created: 29/07/2014
* Author: James Buchanan
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* Prints out all files in active directory */
int wmain(int argc, wchar_t *argv[]){
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t *sDir;
	wchar_t *suffix;
	size_t first_len;
	size_t second_len;
	int i = 0;

	//Help message, printed with -h
	while (argv[i]){
		if ((wcscmp(argv[i], L"-help") == 0) || (wcscmp(argv[i], L"-h") == 0)) {
			printf("ls\tLists the contents of a directory.\n\tUsage: ls [directory] [-h]");
			return EXIT_SUCCESS;
		}
		i++;
	}

	if (argc > 1){
		first_len = wcslen(argv[1]) + 1;
		suffix = L"\\*.*";
		second_len = wcslen(suffix) + 1;
		sDir = malloc(sizeof(wchar_t)* (first_len + second_len));
		wcsncpy(sDir, argv[1], first_len);
		wcsncat(sDir, suffix, second_len);
	}
	else {
		sDir = L".\\*.*";
	}

	if ((hFind = FindFirstFile(sDir, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		fwprintf(stderr, L"Path not found: [%s]\n", sDir);
		return NULL;
	}
	do {
		{
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				wprintf(L"*%s\n", fdFile.cFileName);
			}
			else {
				wprintf(L"%s\n", fdFile.cFileName);
			}
		}
	} while (FindNextFile(hFind, &fdFile));
}
	