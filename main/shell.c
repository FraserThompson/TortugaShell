/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 300
#define MAX_WORD 64
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
HANDLE CONSOLE_OUTPUT;
HANDLE CONSOLE_INPUT;
int CONSOLE_TRANSPARENCY;


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
* Prints a wchar string to a location in the CONSOLE_OUTPUT with a specific set of attributes
* Params: Wchar string to print, handle of CONSOLE_OUTPUT, x coord, y coord, attributes
*/
void advPrint(wchar_t *content, HANDLE CONSOLE_OUTPUT, int x, int y, WORD attributes){
	COORD coords;
	COORD oldCoords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
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

	SetConsoleCursorPosition(CONSOLE_OUTPUT, coords);
	SetConsoleTextAttribute(CONSOLE_OUTPUT, attributes);
	wprintf(L"%s", content);
	SetConsoleCursorPosition(CONSOLE_OUTPUT, oldCoords);
}

/* -----WINDOWS----
* Clears a space at the top and prints the wchar content string in the middle of it.
* Params: Wchar string to print, handle of CONSOLE_OUTPUT
*/
void printHeader(wchar_t *content, HANDLE CONSOLE_OUTPUT){
	int width;
	int len;
	int center_x;
	COORD coords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	DWORD written;
	WORD attributes = (FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE);

	// Work out where to put stuff
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	width = screen_info.dwSize.X;
	len = wcslen(content);
	center_x = width/2 - len/2;
	coords.Y = screen_info.srWindow.Top;
	coords.X = 0;

	// Blank the top
	FillConsoleOutputAttribute(CONSOLE_OUTPUT, attributes, width, coords, &written);
	FillConsoleOutputCharacterW(CONSOLE_OUTPUT, L' ', width, coords, &written);

	// Print the header
	advPrint(content, CONSOLE_OUTPUT, center_x, coords.Y, attributes);
}

static int drawPrompt(void) {
	wchar_t *top = concat_string(L"", getCWD(), L"\n");
	SetConsoleTextAttribute(CONSOLE_OUTPUT, (FOREGROUND_INTENSITY));
	wprintf(L"\n>");
	printHeader(top, CONSOLE_OUTPUT);
	SetConsoleTextAttribute(CONSOLE_OUTPUT, (FOREGROUND_INTENSITY));
}


static COORD getCursor(){
	CONSOLE_SCREEN_BUFFER_INFO cursor_position;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &cursor_position);
	return cursor_position.dwCursorPosition;
}

static COORD moveCursor(int x, int y) {
	COORD coords;
	CONSOLE_SCREEN_BUFFER_INFO cursor_position;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &cursor_position);
	coords = cursor_position.dwCursorPosition;
	coords.Y += y;
	coords.X += x;
	SetConsoleCursorPosition(CONSOLE_OUTPUT, coords);
	return coords;
}

/* -----WINDOWS----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static wchar_t **readline(int *num_words) {
	wchar_t **line = emalloc(sizeof(wchar_t *) * 64);
	wchar_t *word_buffer = emalloc(sizeof(wchar_t) * MAX_WORD);
	wchar_t wcs_buffer = emalloc(sizeof(wchar_t));
	DWORD num_read;
	DWORD backspace_buff;
	WORD colours = FOREGROUND_GREEN;
	int k = 0;
	int word_len = 0;
	int count = 0;
	COORD cursor_loc = getCursor();

	while (1){
		wcs_buffer = getwchar();

		// Enter
		if (wcs_buffer == L'\r'){

			break;
		}

		// Backspace
		if (wcs_buffer == L'\b'){
			cursor_loc = getCursor();

			// Don't let them remove the arrow
			if (cursor_loc.X > 1){
				cursor_loc.X -= 1;
				WriteConsoleOutputCharacter(CONSOLE_OUTPUT, L" ", 1, cursor_loc, &backspace_buff);
				// If we encounter a space, remove words from line and decrease count
			}
			else {
				cursor_loc = moveCursor(1, 0);
			}
		}

		putwchar(wcs_buffer);

		// Space
		if (wcs_buffer == L' '){
			word_buffer[k] = '\0';
			line[count] = malloc(sizeof(wchar_t) * k);
			wcscpy(line[count++], word_buffer);
			line[count] = 0;

			if (count == 1){
				if (wcscmp(word_buffer, L"cwd") == 0){
					cursor_loc = getCursor();
					cursor_loc.X -= k + 1;
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, colours, k, cursor_loc, &num_read);
					cursor_loc.X += k + 1;
				}
			}

			memset(word_buffer, 0, sizeof(word_buffer));
			k = -1;
		}

		word_buffer[k++] = wcs_buffer;
	}

	/*if (fgetws(line, MAX_LINE, stdin) == NULL) {
		fwprintf(stderr, L"READLINE: Error reading line!\n");
		exit(EXIT_FAILURE);
	}*/

	// Add any leftover words
	word_buffer[k] = '\0';
	line[count] = malloc(sizeof(wchar_t)* k);
	wcscpy(line[count++], word_buffer);
	line[count] = 0;

	// Move cursor down a line
	cursor_loc = moveCursor(0, 1);

	*num_words = count;
	return line;
	
}


/* 
* Main loop. Reads a line and parses it.
*/
int wmain(int argc, wchar_t *argv[]) {
	int i = 0;
	int num_words;
	wchar_t *cwd = getCWD();
	wchar_t **line;
	HWND ConsoleWindow;

	// Get the current working directory so we have a reference
	size_t cwd_len = wcslen(getCWD()) + 1;
	PATH = malloc(sizeof(wchar_t)* cwd_len);
	wcscpy(PATH, cwd);

	// Get standard handles
	CONSOLE_OUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_INPUT = GetStdHandle(STD_INPUT_HANDLE);
	ConsoleWindow = GetConsoleWindow();
	SetConsoleTitle(L"Tortuga");
	SetConsoleMode(CONSOLE_INPUT, (ENABLE_MOUSE_INPUT));
	SetConsoleMode(CONSOLE_OUTPUT, (ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT));

	// Adding transperancy
	CONSOLE_TRANSPARENCY = 240;
	SetWindowLong(ConsoleWindow, GWL_EXSTYLE, GetWindowLong(ConsoleWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(ConsoleWindow, 0, CONSOLE_TRANSPARENCY, LWA_ALPHA);


	//PeekConsoleInput(CONSOLE_INPUT, &buffer, 1, &events);
	// Check for debug flag
	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}
	
	while (1) {
		drawPrompt();
		line = readline(&num_words);
		wprintf(L"%i ", num_words);
		parse(line, num_words);
	}

	free(PATH);
	free(line);
	return EXIT_SUCCESS;
}
