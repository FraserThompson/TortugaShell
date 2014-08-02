/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 300
#define MAX_WORD 64
#define NUM_COMMANDS 3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include "process_mgmt.h"
#include "parser.h"
#include "myStrings.h"
#include "shell.h"
#include "cwd.h"
#include "console.h"

int debug_global = 1;
wchar_t *PATH;
HANDLE CONSOLE_OUTPUT;
HANDLE CONSOLE_INPUT;
int CONSOLE_TRANSPARENCY;
WORD HEADER_FOOTER_ATTRIBUTES;
WORD NORMAL_ATTRIBUTES;
WORD PROMPT_ATTRIBUTES;
node *command_tree;

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

/* -----CROSS-PLATFORM----
* Inits a command line struct.
* Return: Allocated and initialized pointer to command line struct.
*/
command_line *init_command_line(wchar_t *command, wchar_t *params, wchar_t *pipe, wchar_t *redirectIn, wchar_t *redirectOut, int type){
	command_line *line = emalloc(sizeof(command_line));

	if (params){
		line->params = _wcsdup(params);
	}
	else {
		line->params = NULL;
	}

	if (command != NULL){
		line->command = _wcsdup(command);
	}
	else {
		line->command = NULL;
	}

	if (pipe != NULL){
		line->pipe = _wcsdup(pipe);
	}
	else {
		line->pipe = NULL;
	}

	if (redirectIn != NULL){
		line->redirectIn = _wcsdup(redirectIn);
	}
	else {
		line->redirectIn = NULL;
	}

	if (redirectOut != NULL){
		line->redirectOut = _wcsdup(redirectOut);
	}
	else {
		line->redirectOut = NULL;
	}
	
	line->type = type;
	line->output = NULL;

	return line;
}

/* -----CROSS-PLATFORM----
* Frees a two dimensional array
* Params: Array to free, last index in array
*/
void free_word_array(wchar_t **cmdline, int last_index){
	int i = 0;
	if (debug_global > 1) wprintf(L"FREE_WORD_ARRAY: Freeing %i items...\n", last_index);
	free(cmdline[i++]);

	while (i < last_index - 1){
		free(cmdline[i]);
		i++;
	}

}

/* -----CROSS-PLATFORM----
* Frees a command_line struct
* Param: Struct to free
*/
void free_command_line(command_line *line){
	if (debug_global > 1) wprintf(L"FREE_COMMAND_LINE: Freeing struct\n");

	if (line->command){
		free(line->command);
	}
	if (line->output){
		free(line->output);
	}
	if (line->params){
		free(line->params);
	}
	if (line->pipe){
		free(line->pipe);
	}
	if (line->redirectIn){
		free(line->redirectIn);
	}
	if (line->redirectOut){
		free(line->redirectOut);
	}
	free(line);
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
	else if (y == 0){
		coords.Y = getConsoleTop();
		coords.X = x;
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
static void printHeader(wchar_t *content){
	int width = getConsoleWidth();
	int len;
	int center_x;
	COORD topCoords;
	DWORD written;

	// Work out where to put stuff
	len = wcslen(content);
	center_x = width / 2 - len / 2;
	topCoords.Y = getConsoleTop();
	topCoords.X = 0;

	clearLine(width, topCoords.X, topCoords.Y, HEADER_FOOTER_ATTRIBUTES);

	// Print the text
	if (content != NULL){
		advPrint(content, CONSOLE_OUTPUT, center_x, topCoords.Y, HEADER_FOOTER_ATTRIBUTES);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, NORMAL_ATTRIBUTES);
}

/* -----WINDOWS----
* Clears a space at the bottom and prints the wchar content string in the middle of it.
* Params: Wchar string to print, handle of CONSOLE_OUTPUT
*/
static void printFooter(wchar_t *content){
	COORD bottomCoords;
	DWORD written;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	int width = getConsoleWidth();

	// Work out where to put stuff
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	bottomCoords.X = 0;
	bottomCoords.Y = getConsoleBottom() - 2;

	clearLine(width, bottomCoords.X, bottomCoords.Y, NORMAL_ATTRIBUTES);

	// Print the text
	if (content != NULL){
		advPrint(content, CONSOLE_OUTPUT, 0, bottomCoords.Y, HEADER_FOOTER_ATTRIBUTES);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, NORMAL_ATTRIBUTES);
}

/* -----WINDOWS----
* Draws a prompt at the top containing the cwd
*/
static void drawPrompt(void) {
	wchar_t *top = concat_string(L"", getCWD(), L"\n");
	COORD current_cursor = getCursor();
	int height = getConsoleHeight();
	int width = getConsoleWidth();

	// If we're going to be printing over the footer then move everything down
	if (current_cursor.Y >= height - 2){

		//Clear footer
		clearLine(width, 0, height, NORMAL_ATTRIBUTES);

		//Clear header
		clearLine(width, 0, 0, NORMAL_ATTRIBUTES);

		// Move cursor down to scroll then back up for typing
		current_cursor = moveCursor(0, 3, -1, -1);
		current_cursor = moveCursor(0, -4, -1, -1);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, PROMPT_ATTRIBUTES);
	wprintf(L"\n>");
	printHeader(top);
	printFooter(L"Start typing to begin...");
}

/* -----WINDOWS----
* Highlights known commands and prints usage tips
* Return: 1 if match
* Param: Command to check, count of characters in that command
*/
static int highlight_command(wchar_t *command, int wordchar_count){
	COORD cursor_loc = getCursor();
	WORD colours = FOREGROUND_GREEN;
	DWORD num_read;
	node *parent;
	node *result = bst_search(command_tree, command, &parent);

	if (does_file_exist(command)){
		cursor_loc.X -= wordchar_count;
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, colours, wordchar_count + 1, cursor_loc, &num_read);
	}

	if (result != NULL){
		cursor_loc.X -= wordchar_count;
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, colours, wordchar_count + 1, cursor_loc, &num_read);
		printFooter(result->description);
	}


	return 1;
}

/* -----WINDOWS----
* Check to see if a file exists
* Return: 1 if exists
* Param: Path of file to check
*/
static int does_file_exist(wchar_t *command){
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(command, &FindFileData);
	int found = handle != INVALID_HANDLE_VALUE;
	if (found)
	{
		FindClose(handle);
	}
	return found;
}


/* -----WINDOWS----
* Prints a prompt, interprets user input.
* Return: Line that the user inputted
*/
static wchar_t **readline(int *num_words) {
	wchar_t **line_array; //the completed line split into an array
	wchar_t *word_buffer = emalloc(sizeof(wchar_t) * MAX_WORD); //holds each space seperated word
	wchar_t *line_buffer = emalloc(sizeof(wchar_t) * MAX_LINE); //holds the entire line
	wchar_t backspace_buffer = emalloc(sizeof(wchar_t)); //buffers the character removed by the backspace
	backspace_buffer = NULL;
	wint_t wcs_buffer = emalloc(sizeof(wchar_t)); //buffers each character typed
	DWORD num_read;
	DWORD backspace_read;
	COORD cursor_loc;
	COORD cursor_orig = getCursor();
	int listening = 1;
	int count;
	int end_of_line;
	int width = getConsoleWidth();
	int bottom = getConsoleBottom();
	int k = 0; //number of charactres in line array
	int word_count = 0; //number of words
	int wordchar_count = 0; //number of characters in current word

	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf[128];
	int counter = 0;
	wchar_t intstr[3];

	do {
		wcs_buffer = getch(); //this is non standard but all modern windows compilers should have it and it's the only way I've found to fix the double enter issue which happens with getwchar
		switch (wcs_buffer){

		case L'\r':
			if (k != 0){
				listening = 0;
			}
			break;

		case L'\b':
			cursor_loc = getCursor();
			// Only backspace if we're within the bounds
			if (cursor_loc.X > 1 || cursor_loc.Y > cursor_orig.Y){
				end_of_line = cursor_loc.X == 0 ? 1 : 0;
				backspace_buffer = line_buffer[k - 1];
				line_buffer[k--] = L'\0';

				if (wordchar_count != 0){
					word_buffer[wordchar_count--] = L'\0';
				}

				// Moving cursor if we've wrapped around
				if (end_of_line){
					cursor_loc.X = width;
					cursor_loc.Y -= 1;
				}

				cursor_loc.X -= 1;

				WriteConsoleOutputCharacter(CONSOLE_OUTPUT, L" ", 1, cursor_loc, &backspace_read);

				if (debug_global) {
					swprintf(intstr, 3, L"%d", k);
					advPrint(intstr, CONSOLE_OUTPUT, 2, 0, NULL);
				}

				// If there's a space we should remove from word_count
				if (backspace_buffer == L' '){
					word_count--;
					if (debug_global) {
						swprintf(intstr, 3, L"%d", word_count);
						advPrint(intstr, CONSOLE_OUTPUT, 0, 0, NULL);
					}
				}
			}
			else {
				cursor_loc = moveCursor(1, 0, -1, -1);
			}

			putwchar(wcs_buffer);

			// Move the cursor to the right place if we've wrapped around
			if (end_of_line){
				SetConsoleCursorPosition(CONSOLE_OUTPUT, cursor_loc);
			}

			// Blank any possible usage tips
			if (word_count == 0){
				clearLine(width * 3, 0, bottom - 2, NORMAL_ATTRIBUTES);
			}
			break;

		case L' ':
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

			if (word_count == 0){
				// Blank any possible usage tips
				clearLine(width * 3, 0, bottom - 2, NORMAL_ATTRIBUTES);

				// Check to see if the command is recognized
				word_buffer[wordchar_count] = L'\0';
				highlight_command(word_buffer, wordchar_count);
			}

			if (debug_global) {
				swprintf(intstr, 3, L"%d", k);
				advPrint(intstr, CONSOLE_OUTPUT, 2, 0, NULL);
			}
			break;
		}
	} while (listening && k < MAX_LINE - 1);

	FlushConsoleInputBuffer(CONSOLE_INPUT);

	// Add null termination
	line_buffer[k] = L'\0';

	// Clear header/footer
	clearLine(width, 0, 0, NORMAL_ATTRIBUTES);
	clearLine(width * 3, 0, bottom - 2, NORMAL_ATTRIBUTES);

	// Move cursor down a line
	cursor_loc = moveCursor(0, 1, 0, -1);

	// Split into array
	line_array = split(line_buffer, L" ", &count);
	*num_words = count;

	free(word_buffer);
	free(line_buffer);

	return line_array;
}

/* -----WINDOWS----
* Scans the ./commands/ directory and makes a node containing the name of each exe and what it prints when called with -h. 
* Return: Root node of BST 
*/
static node *build_command_tree(void){
	node *newnode, *temp, *parent;
	node *root = NULL;
	wchar_t *commandsDir = concat_string(PATH, L"\\commands", NULL);
	wchar_t *sDir = concat_string(commandsDir, L"\\*.exe", NULL);
	wchar_t sPath[2048];
	wchar_t *result;
	wchar_t *command_name;
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	command_line *line = init_command_line(NULL, L"-h", NULL, NULL, L":var:", 1);
	int error = 0;
	int debug_old = debug_global;
	debug_global = 0;
	wchar_t *recognized_commands[NUM_COMMANDS] = { L"cwd", L"help", L"cd" };
	wchar_t *command_usage[NUM_COMMANDS] = { L"cwd\tPrints the current working directory.\n\tUsage: cwd [directory] [-h]", L"help\tPrints a list of possible commands.\n\tUsage: help", L"cd\tChanges the current working directory.\n\tUsage: cd [directory] [-h]" };

	// First add built in commands
	for (int i = 0; i < NUM_COMMANDS; i++){
		newnode = init_node();
		newnode->title = _wcsdup(recognized_commands[i]);
		newnode->description = _wcsdup(command_usage[i]);

		if (root == NULL){
			root = newnode;
		}
		else {
			bst_insert(root, newnode);
		}
	}

	// Now search ./commands/ 
	if ((hFind = FindFirstFile(sDir, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Path not found: [%s]\n", sDir);
		return NULL;
	}

	do
	{
		{
			wsprintf(sPath, L"%s\\%s", commandsDir, fdFile.cFileName);

			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}
			else {
				newnode = init_node();

				// Cut off command extension
				command_name = fdFile.cFileName;
				command_name[wcslen(fdFile.cFileName) - 4] = 0;

				// Copy to command node
				newnode->title = _wcsdup(command_name);

				if (debug_global) wprintf(L"Found command: %s\n", newnode->title);

				// Copy command to commandline struct for process calling
				line->command = _wcsdup(sPath);

				error = create_process(line);

				if (error != 0) {
					fwprintf(stderr, L"BUILD_COMMAND_TREE: Could not open.\n");
					return;
				}

				// Copy help description
				newnode->description = _wcsdup(line->output);

				bst_insert(root, newnode);
			}
		}
	} while (FindNextFile(hFind, &fdFile));
	FindClose(hFind);

	debug_global = debug_old;

	free(commandsDir);
	free(sDir);
	//free_command_line(line); //why doesn't this work?

	return root;
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
	SetConsoleMode(CONSOLE_INPUT, (ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT));

	// Style stuff
	CONSOLE_TRANSPARENCY = 240;
	SetWindowLong(ConsoleWindow, GWL_EXSTYLE, GetWindowLong(ConsoleWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(ConsoleWindow, 0, CONSOLE_TRANSPARENCY, LWA_ALPHA);
	HEADER_FOOTER_ATTRIBUTES = (FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE);
	NORMAL_ATTRIBUTES = (FOREGROUND_INTENSITY);
	PROMPT_ATTRIBUTES = (FOREGROUND_RED);

	// Check for debug flag
	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	// Build command BST from ./commands directory
	command_tree = build_command_tree();
	assert(command_tree != NULL);

	// Main loop
	while (1) {
		drawPrompt();
		line = readline(&num_words);
		assert(num_words > 0);
		parse(line, num_words);
	}

	free(PATH);
	free(line);
	bst_free(command_tree);

	return EXIT_SUCCESS;
}
