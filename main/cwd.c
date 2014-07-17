/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*
*  Returns the path to the current working directory in Windows.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "myStrings.h"
#include "cwd.h"

/* -------WINDOWS------
* Returns the CWD in windows
*/
wchar_t *getCWD(){
	wchar_t cwd[1024];

	if (!GetCurrentDirectoryW(1024, cwd)) {
		fwprintf(stderr, L"CWD: Fatal error getting current directory! %s\n", GetLastError());
		exit(EXIT_FAILURE);
	}

	return cwd;
}