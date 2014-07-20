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
#include <Windows.h>
#include "parser.h"
#include "myStrings.h"
#include "shell.h"
#include "cwd.h"

int debug_global = 1;
wchar_t *PATH;
HANDLE CONSOLE;

/* -----CROSS-PLATFORM----
* Malloc with error checking.
* Return: Allocated memory 
*/
void *emalloc(size_t size){
	void *p = malloc(size);
	if (p == NULL){
		fwprintf(stderr, L"EMALLOC: Fatal error! Ran out of memory!\n");
		exit(1);
	}
	return p;
}

/* -----CROSS-PLATFORM----
* Realloc with error checking.
* Return: Allocated memory
*/
void *erealloc(void *ptr, size_t size){
	void *p = realloc(ptr, size);
	if (p == NULL){
		fwprintf(stderr, L"EMALLOC: Fatal error! Ran out of memory!\n");
		exit(1);
	}
	return p;
}

/* -----WINDOWS----
* Prints a wchar string to a location in the console with a specific set of attributes
* Params: Wchar string to print, handle of console, x coord, y coord, attributes
*/
void advPrint(wchar_t *content, HANDLE console, int x, int y, WORD attributes){
	COORD coords;
	COORD oldCoords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(CONSOLE, &screen_info);
	oldCoords.X = screen_info.dwCursorPosition.X;
	oldCoords.Y = screen_info.dwCursorPosition.Y;

	// If x and y are -1 then we should just print at the current location
	if (x == -1 || y == -1){
		coords.X = oldCoords.X;
		coords.Y = oldCoords.Y;
	}
	else {
		coords.X = x;
		coords.Y = y;
	}

	// If no attributes are supplied then it's a darkish gray
	if (attributes == NULL){
		attributes = (FOREGROUND_INTENSITY);
	}

	SetConsoleCursorPosition(CONSOLE, coords);
	SetConsoleTextAttribute(CONSOLE, attributes);
	wprintf(L"%s", content);
	SetConsoleCursorPosition(CONSOLE, oldCoords);
}

/* -----WINDOWS----
* Clears a space at the top and prints the wchar content string in the middle of it.
* Params: Wchar string to print, handle of console
*/
void printHeader(wchar_t *content, HANDLE console){
	int width;
	int len;
	int center_x;
	COORD coords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	DWORD written;
	WORD attributes = (FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE);

	// Work out where to put stuff
	GetConsoleScreenBufferInfo(CONSOLE, &screen_info);
	width = screen_info.dwSize.X;
	len = wcslen(content);
	center_x = width/2 - len/2;
	coords.Y = screen_info.srWindow.Top;
	coords.X = 0;

	// Blank the top
	FillConsoleOutputAttribute(CONSOLE, attributes, width, coords, &written);
	FillConsoleOutputCharacterW(CONSOLE, L' ', width, coords, &written);

	// Print the header
	advPrint(content, CONSOLE, center_x, coords.Y, attributes);
}

/* -----WINDOWS----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static wchar_t *readline(void) {
	wchar_t *line = emalloc(sizeof(wchar_t) * MAX_LINE);
	wchar_t *top = concat_string(L"", getCWD(), L"\n");

	SetConsoleTextAttribute(CONSOLE, (FOREGROUND_INTENSITY));
	wprintf(L"\n>");

	printHeader(top, CONSOLE);
	SetConsoleTextAttribute(CONSOLE, (FOREGROUND_INTENSITY));

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
int wmain(int argc, wchar_t *argv[]) {
	int i = 0;
	CONSOLE = GetStdHandle(STD_OUTPUT_HANDLE);
	wchar_t *cwd = getCWD();
	size_t cwd_len = wcslen(getCWD()) + 1;
	PATH = malloc(sizeof(wchar_t) * cwd_len);
	wcscpy(PATH, cwd);

	SetConsoleTitle(L"Tortuga");

	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	while (1){
		parse(readline());
	}

	return EXIT_SUCCESS;
}
