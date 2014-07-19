/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 300
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "shell.h"
#include "cwd.h"

int debug_global = 0;
wchar_t *PATH;

/* -----CROSS-PLATFORM----
* Malloc with error checking.
* Return: Allocated memory 
*/
void *emalloc(size_t size){
	void *p = malloc(size);
	if (p == NULL){
		fwprintf(stderr, "EMALLOC: Fatal error! Ran out of memory!\n");
		exit(1);
	}
	return p;
}

/* -----CROSS-PLATFORM----
* Realloc with error checking.
* Return: Allocated memory
*/
void *erealloc(size_t size, void *ptr){
	void *p = realloc(size, ptr);
	if (p == NULL){
		fwprintf(stderr, "EMALLOC: Fatal error! Ran out of memory!\n");
		exit(1);
	}
	return p;
}

/* -----WINDOWS----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static wchar_t *readline(void) {
	wchar_t *line = emalloc(sizeof(wchar_t) * MAX_LINE);
	wprintf(L"%s>", getCWD());

	if (fgetws(line, MAX_LINE, stdin) == NULL) {
		fwprintf(stderr, L"READLINE: Error reading line!\n");
		exit(EXIT_FAILURE);
	}
	else {
		return line;
	}
}

/* 
* Main loop. Reads a line and parses it.
*/
int main(int argc, char *argv[]) {
	int i = 0;
	wchar_t *cwd = getCWD();
	size_t cwd_len = wcslen(getCWD()) + 1;
	PATH = malloc(sizeof(wchar_t) * cwd_len);
	wcscpy(PATH, cwd);

	while (argv[i]){
		if ((strcmp(argv[i], L"-d") == 0) || (strcmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	while (1){
		parse(readline());
	}

	return EXIT_SUCCESS;
}
