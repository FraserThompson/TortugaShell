/*
* cd.c
*
*  Created on: 17/05/2014
*      Author: Fraser
*
*  Changes the current working directory.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "shell.h"
#include "cd.h"

/* -----CROSS-PLATFORM----
* Changes directory.
*/
void cd(wchar_t *dir) {
	int i = 0;

	if (_wchdir(dir) != 0||dir==NULL) {
		wprintf(L"CD: Error! Malformed path maybe?\n");
		return;
	}

	if (debug_global){ wprintf(L"CD: Changed directory to %s.\n", dir); }
}

