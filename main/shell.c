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

/* -----WINDOWS----
* Draws a prompt at the top containing the cwd
*/
static void drawPrompt(void) {
	wchar_t *top = concat_string(L"", getCWD(), L"\n");
	SetConsoleTextAttribute(CONSOLE_OUTPUT, (FOREGROUND_INTENSITY));
	wprintf(L"\n>");
	printHeader(top, CONSOLE_OUTPUT);
	SetConsoleTextAttribute(CONSOLE_OUTPUT, (FOREGROUND_INTENSITY));
}

/* -----WINDOWS----
* Gets the current console cursor position
* Return: Cursor position
*/
static COORD getCursor(){
	CONSOLE_SCREEN_BUFFER_INFO cursor_position;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &cursor_position);
	return cursor_position.dwCursorPosition;
}

/* -----WINDOWS----
* Moves the console cursor to a specified position relative to current
* Params: num of rows/columns to move
* Return: The new position
*/
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
	wchar_t **line_array;
	wchar_t *word_buffer = emalloc(sizeof(wchar_t) * MAX_WORD); //holds a word
	wchar_t *line_buffer = emalloc(sizeof(wchar_t) * MAX_LINE); //holds the entire line
	wchar_t wcs_buffer = malloc(sizeof(wchar_t)); //buffers each character typed
	wchar_t backspace_buffer = malloc(sizeof(wchar_t)); //buffers the character removed by the backspace
	DWORD num_read;
	DWORD backspace_read;
	WORD colours = FOREGROUND_GREEN;
	COORD cursor_loc = getCursor();
	int listening = 1;
	int count;
	int k = 0; //number of charactres in line array
	int word_count = 0; //number of words
	int wordchar_count = 0; //number of characters in current word

	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf[128];
	int counter = 0;
	wchar_t intstr[3];

	FlushConsoleInputBuffer(CONSOLE_INPUT);

	while (listening){
		wcs_buffer = getwchar();
		switch (wcs_buffer){

		case L'':
			break;

		case 13:
			listening = 0;
			break;

		case L'\b':
			cursor_loc = getCursor();

			// Only backspace if we're within the bounds
			if (cursor_loc.X > 1){
				cursor_loc.X -= 1;
				backspace_buffer = line_buffer[k];
				line_buffer[k--] = 0;
				if (backspace_buffer == L' '){
					word_count--;
					if (debug_global) {
						swprintf(intstr, 3, L"%d", word_count);
						advPrint(intstr, CONSOLE_OUTPUT, 0, 0, NULL);
					}
				}
				WriteConsoleOutputCharacter(CONSOLE_OUTPUT, L" ", 1, cursor_loc, &backspace_read);
			}
			else {
				cursor_loc = moveCursor(1, 0);
			}

			putwchar(wcs_buffer);
			break;

		case L' ':
			// Finish the word off
			word_buffer[wordchar_count] = L'\0';

			// If we're on the first word check to see if it's cwd
			if (word_count == 0){
				if (wcscmp(word_buffer, L"cwd") == 0){
					if (debug_global > 1) advPrint(L"GOT CWD", CONSOLE_OUTPUT, k, 0, NULL);
					cursor_loc = getCursor();
					cursor_loc.X -= wordchar_count;
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, colours, wordchar_count, cursor_loc, &num_read);
					cursor_loc.X += wordchar_count;
				}
			}

			word_count++;

			if (debug_global) {
				swprintf(intstr, 3, L"%d", word_count);
				advPrint(intstr, CONSOLE_OUTPUT, 0, 0, NULL);
			}

			// Empty the word
			wordchar_count = 0;
			memset(word_buffer, 0, wordchar_count);
			line_buffer[k++] = wcs_buffer;
			putwchar(wcs_buffer);
			break;

		default:
			line_buffer[k++] = wcs_buffer;
			word_buffer[wordchar_count++] = wcs_buffer;
			putwchar(wcs_buffer);
			break;
		}
	}

	/*if (fgetws(line, MAX_LINE, stdin) == NULL) {
		fwprintf(stderr, L"READLINE: Error reading line!\n");
		exit(EXIT_FAILURE);
	}*/


	// Add null termination
	line_buffer[k] = L'\0';

	// Move cursor down a line
	cursor_loc = moveCursor(0, 1);

	// Split into array
	line_array = split(line_buffer, L" ", &count);
	*num_words = count;

	return line_array;
	
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

	// Get the current working directory
	size_t cwd_len = wcslen(getCWD()) + 1;
	PATH = malloc(sizeof(wchar_t)* cwd_len);
	wcscpy(PATH, cwd);

	// Get standard handles
	CONSOLE_OUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_INPUT = GetStdHandle(STD_INPUT_HANDLE);
	ConsoleWindow = GetConsoleWindow();
	SetConsoleTitle(L"Tortuga");
	SetConsoleMode(CONSOLE_INPUT, (ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT));

	// Adding transperancy
	CONSOLE_TRANSPARENCY = 240;
	SetWindowLong(ConsoleWindow, GWL_EXSTYLE, GetWindowLong(ConsoleWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(ConsoleWindow, 0, CONSOLE_TRANSPARENCY, LWA_ALPHA);

	// Check for debug flag
	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}
	
	// Main loop
	while (1) {
		drawPrompt();
		line = readline(&num_words);
		parse(line, num_words);
	}

	free(PATH);
	free(line);
	return EXIT_SUCCESS;
}
