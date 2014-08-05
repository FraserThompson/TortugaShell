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
	wchar_t *sDir = L".\\*.*";

	//wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);
	if ((hFind = FindFirstFile(sDir, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return NULL;
	}
	do {
		{
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}
			else {
				wprintf(L"%s\n", fdFile.cFileName);
			}
		}
	} while (FindNextFile(hFind, &fdFile));
	//system("pause");
}
	