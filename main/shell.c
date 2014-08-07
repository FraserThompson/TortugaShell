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
#define NUM_ATTRIBUTES 6
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <conio.h>
#include "process_mgmt.h"
#include "parser.h"
#include "myStrings.h"
#include "shell.h"
#include "cwd.h"
#include "console.h"

int debug_global = 1;
int found = 0; // used as a flag for highlight_command to denote whether we need to build another directory bst
int CONSOLE_TRANSPARENCY;
wchar_t *PATH;
HANDLE CONSOLE_OUTPUT;
HANDLE CONSOLE_INPUT;
int HEADER_FOOTER_ATTRIBUTES;
int NORMAL_ATTRIBUTES;
int PROMPT_ATTRIBUTES;
int DIR_HIGHLIGHT_ATTRIBUTES;
int FILE_HIGHLIGHT_ATTRIBUTES;
int TAB_SUGGESTION_ATTRIBUTES;
WORD POSSIBLE_ATTRIBUTES[NUM_ATTRIBUTES] = { (FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE), (FOREGROUND_INTENSITY), (FOREGROUND_RED), (FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),
(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY), (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED) };
wchar_t *TAB_SUGGESTION; //contains the complete tab suggestion
node *command_tree; //contains the tree of ./commands
node *current_dir_tree; //contains the tree of the current directory the user is typing


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
* Check to see if a file exists
* Return: 1 if it's a file, 2 if it's a directory
* Param: Path of file to check
*/
static int does_file_exist(wchar_t *command){
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(command, &FindFileData);
	int found = handle != INVALID_HANDLE_VALUE;
	int result = 0;

	if (found)
	{
		result = 1;
		if (FindFileData.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
		{
			result = 2;
		}
		FindClose(handle);
	}

	return result;
}

/* -----WINDOWS----
* Builds a BST from a directory.
* Param: Directory to build from (without trailing slash)
* Return: Completed tree
*/
static node *tree_from_dir(wchar_t *dir){
	node *newnode;
	node *root = NULL;
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	wchar_t *sDir = concat_string(dir, L"\\*.*", NULL);

	if ((hFind = FindFirstFile(sDir, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	do
	{
		{
			if (wcscmp(fdFile.cFileName, L".") == 0 || wcscmp(fdFile.cFileName, L"..") == 0){
				continue;
			}
			newnode = init_node();

			// Copy to command node
			newnode->title = concat_string(dir, L"\\", fdFile.cFileName);
			newnode->description = NULL;

			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				newnode->type = 1;
				newnode->title = concat_string(newnode->title, L"\\", NULL);
			}

			if (root == NULL){
				root = newnode;
			}
			else {
				bst_insert(root, newnode);
			}

		}
	} while (FindNextFile(hFind, &fdFile));
	FindClose(hFind);

	return root;
}

/* -----WINDOWS----
* Scans the ./commands/ directory and makes a node containing the name of each exe and what it prints when called with -h.
* Return: Root node of BST
*/
static node *build_command_tree(void){
	node *newnode;
	node *root = NULL;
	wchar_t *commandsDir = concat_string(PATH, L"\\commands", NULL);
	wchar_t *sDir = concat_string(commandsDir, L"\\*.exe", NULL);
	wchar_t sPath[2048];
	wchar_t *command_name;
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
	command_line *line = init_command_line(NULL, L"-h", NULL, NULL, L":var:", 1);
	int error = 0;
	int debug_old = debug_global;
	debug_global = 0;
	wchar_t *recognized_commands[NUM_COMMANDS] = { L"cwd", L"help", L"cd" };
	wchar_t *command_usage[NUM_COMMANDS] = { L"cwd\tPrints the current working directory.\n\tUsage: cwd [directory] [-h]\n", L"help\tPrints a list of possible commands.\n\tUsage: help\n", L"cd\tChanges the current working directory.\n\tUsage: cd [directory] [-h]\n" };

	// First add built in commands
	for (int i = 0; i < NUM_COMMANDS; i++){
		newnode = init_node();
		newnode->title = _wcsdup(recognized_commands[i]);
		newnode->description = _wcsdup(command_usage[i]);
		newnode->type = 2;

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
		fwprintf(stderr, L"\nBUILD_COMMAND_TREE: No ./commands/*.exe files found! Only builtin commands will be available.");
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
				newnode->type = 1;

				if (debug_global) wprintf(L"Found command: %s\n", newnode->title);

				// Copy command to commandline struct for process calling
				line->command = _wcsdup(sPath);

				error = create_process(line);

				if (error != 0) {
					fwprintf(stderr, L"BUILD_COMMAND_TREE: Could not open.\n");
					return NULL;
				}

				// Copy help description
				newnode->description = _wcsdup(line->output);

				bst_insert(root, newnode);

				free_command_line(line);
				line = init_command_line(NULL, L"-h", NULL, NULL, L":var:", 1);
			}
		}
	} while (FindNextFile(hFind, &fdFile));
	FindClose(hFind);

	debug_global = debug_old;

	free(commandsDir);
	free(sDir);
	free_command_line(line);

	return root;
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

	// Work out where to put stuff
	len = wcslen(content);
	center_x = width / 2 - len / 2;
	topCoords.Y = getConsoleTop();
	topCoords.X = 0;

	clearLine(width, topCoords.X, topCoords.Y, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);

	// Print the text
	if (content != NULL){
		advPrint(content, CONSOLE_OUTPUT, center_x, topCoords.Y, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);
}

/* -----WINDOWS----
* Clears a space at the bottom and prints the wchar content string in the middle of it.
* Params: Wchar string to print, handle of CONSOLE_OUTPUT
*/
static void printFooter(wchar_t *content){
	COORD bottomCoords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	int width = getConsoleWidth();

	// Work out where to put stuff
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	bottomCoords.X = 0;
	bottomCoords.Y = getConsoleBottom() - 2;

	clearLine(width, bottomCoords.X, bottomCoords.Y, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

	// Print the text
	if (content != NULL){
		advPrint(content, CONSOLE_OUTPUT, 0, bottomCoords.Y, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);
}

COORD draw_a_box(COORD current_cursor, WORD border_attributes, wchar_t *title, int height, int width, int speed){
	int i = 0;
	int title_len = wcslen(title);
	int center = width / 2 - title_len / 2;

	// Drawing perimeter
	advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	for (i = 0; i < height; i++){
		Sleep(speed);
		current_cursor = moveCursor(0, 1, -1, -1);
		advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	for (i = 0; i < width; i++){
		Sleep(speed);
		current_cursor = moveCursor(1, 0, -1, -1);
		advPrint(L"=", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	for (i = 0; i < height; i++){
		Sleep(speed);
		current_cursor = moveCursor(0, -1, -1, -1);
		advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	for (i = 1; i < width; i++){
		Sleep(speed);
		current_cursor = moveCursor(-1, 0, -1, -1);
		advPrint(L"=", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	//header
	advPrint(title, CONSOLE_OUTPUT, center, current_cursor.Y, border_attributes);
	current_cursor = moveCursor(0, 1, -1, -1);
	return current_cursor;
}

COORD draw_options(wchar_t **options, int *attributes, int num_options, int width, COORD current_cursor){
	int i = 0;
	int option_len = 0;
	int option_center = 0;

	// Drawing options
	for (i = 0; i < num_options; i++){
		option_len = wcslen(options[i]);
		option_center = width / 2 - option_len / 2;
		current_cursor = moveCursor(0, 1, -1, -1);
		advPrint(options[i], CONSOLE_OUTPUT, option_center, current_cursor.Y, POSSIBLE_ATTRIBUTES[attributes[i]]);
	}

	return current_cursor;
}

int drawSettings(){
	COORD current_cursor = getCursor();
	COORD options_cursor = { 1, 1 };
	COORD original_cursor = current_cursor;
	WORD border_attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
	WORD option_attributes = FOREGROUND_INTENSITY | BACKGROUND_BLUE;
	WORD select_attributes = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
	wchar_t *styles_header = L"STYLES";
	wchar_t *options[6] = { L"HEADER FOOTER", L"NORMAL", L"PROMPT", L"DIR HIGHLIGHT", L"FILE HIGHLIGHT", L"TAB SUGGESTION" };
	int attributes[6] = { HEADER_FOOTER_ATTRIBUTES, NORMAL_ATTRIBUTES, PROMPT_ATTRIBUTES, DIR_HIGHLIGHT_ATTRIBUTES, FILE_HIGHLIGHT_ATTRIBUTES, TAB_SUGGESTION_ATTRIBUTES };

	wint_t input;
	wint_t secondInput;

	int i = 0;
	int speed = 5;
	int height = 10;
	int width = 30;
	int num_read;

	current_cursor = draw_a_box(current_cursor, border_attributes, L"STYLES", height, width, speed);
	options_cursor = current_cursor;
	current_cursor = draw_options(options, attributes, 6, width, current_cursor);
	current_cursor = moveCursor(0, -5, -1, -1);

	while (1){
		// Draw the selection cursors;
		advPrint(L">", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
		advPrint(L"<", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
		input = _getch();
		switch (input){
		case 224:
		case 0:
			secondInput = _getch();
			// Down
			if (secondInput == 80){
				if (i < 5){
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					current_cursor = moveCursor(0, 1, -1, -1);
					advPrint(L">", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L"<", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					i++;
				}
			}
			// Up
			else if (secondInput == 72){
				if (i > 0){
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					current_cursor = moveCursor(0, -1, -1, -1);
					advPrint(L">", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L"<", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					i--;

				}
			}
			// Left
			else if (secondInput == 75){
				if (NORMAL_ATTRIBUTES > 0) NORMAL_ATTRIBUTES--;
				attributes[1] = NORMAL_ATTRIBUTES;
				original_cursor = getCursor();
				current_cursor = moveCursor(0, 0, options_cursor.X, options_cursor.Y);
				current_cursor = draw_options(options, attributes, 6, width, current_cursor);
				current_cursor = moveCursor(0, 0, original_cursor.X, original_cursor.Y);
			}
			// Right
			else if (secondInput == 77){
				if (NORMAL_ATTRIBUTES < 5) NORMAL_ATTRIBUTES++;
				attributes[1] = NORMAL_ATTRIBUTES;
				original_cursor = getCursor();
				current_cursor = moveCursor(0, 0, options_cursor.X, options_cursor.Y);
				current_cursor = draw_options(options, attributes, 6, width, current_cursor);
				current_cursor = moveCursor(0, 0, original_cursor.X, original_cursor.Y);
			}
			break;
		}
	}
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
		clearLine(width, 0, height, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

		//Clear header
		clearLine(width, 0, 0, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

		// Move cursor down to scroll then back up for typing
		current_cursor = moveCursor(0, 3, -1, -1);
		current_cursor = moveCursor(0, -4, -1, -1);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[PROMPT_ATTRIBUTES]);
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
	COORD cursor_loc = getCursor(); //current cursor location
	COORD word_begin = cursor_loc; //cursor location at the beginning of the word
	word_begin.X -= wordchar_count;

	//bst stuff
	node *parent;
	node *other_parent;
	node *result = NULL;
	node *other_result = NULL;

	DWORD num_read;
	DWORD written;

	int width = getConsoleWidth();
	int exists;
	int suggestionLen;
	int returnValue = 0;
	TAB_SUGGESTION = NULL;

	if (command[wordchar_count - 1] == '\\'){
		found = 0;
	}

	if (current_dir_tree != NULL){
		other_result = bst_partial_search(current_dir_tree, command, &other_parent);
		if (other_result){
			TAB_SUGGESTION = other_result->title;
			suggestionLen = wcslen(other_result->title);

			// Print and highlight the suggestion
			advPrint(other_result->title, CONSOLE_OUTPUT, word_begin.X, cursor_loc.Y, POSSIBLE_ATTRIBUTES[TAB_SUGGESTION_ATTRIBUTES]);
			FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[DIR_HIGHLIGHT_ATTRIBUTES], wordchar_count, word_begin, &num_read);

			// Clear the rest of the line
			word_begin.X += suggestionLen;
			FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width - cursor_loc.X, word_begin, &written);
		}
		else {
			FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width - cursor_loc.X, cursor_loc, &written);
			FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES], width - cursor_loc.X, cursor_loc, &written);
		}
	}

	// Check to see if it's a known command
	if (command_tree != NULL){
		result = bst_search(command_tree, command, &parent);
	}

	// If it's known print the associated help message and return the command value
	if (result != NULL){
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[FILE_HIGHLIGHT_ATTRIBUTES], wordchar_count + 1, word_begin, &num_read);
		printFooter(result->description);
		returnValue = result->type;
	}
	else {
		// Check to see if it exists
		exists = does_file_exist(command);

		//file
		if (exists == 1){
			FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[FILE_HIGHLIGHT_ATTRIBUTES], wordchar_count + 1, word_begin, &num_read);
			printFooter(L"Press enter to run it");

		}

		//dir
		if (exists == 2){
			FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[DIR_HIGHLIGHT_ATTRIBUTES], wordchar_count + 1, word_begin, &num_read);
			if (!found){
				current_dir_tree = tree_from_dir(command);
				found = 1;
			}
		}
	}

	return returnValue;
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
	wint_t wcs_buffer;
	DWORD backspace_read;
	COORD cursor_loc;
	COORD cursor_orig = getCursor(); //location of the cursor before anything has happened
	int count;
	int end_of_line = 0;
	int width = getConsoleWidth();
	int bottom = getConsoleBottom();
	int listening = 1; //used to end the while loop
	int k = 0; //number of charactres in line array
	int word_count = 0; //number of words
	int wordchar_count = 0; //number of characters in current word
	int recognized_command = 0;
	wchar_t intstr[3];
	found = 0;
	backspace_buffer = 0;

	do {
		wcs_buffer = _getwch(); //this is worse than getwchar() but gets around the stupid double enter bug
		switch (wcs_buffer){
		
		// Discard arrows/escape
		case 224:
		case 0:
			_getwch();
		case 27:
			break;

		case L'\t':
			if (TAB_SUGGESTION){
				cursor_loc = getCursor();

				// Copy all the relevant things to the places
				wcscpy(line_buffer, TAB_SUGGESTION);
				wcscpy(word_buffer, TAB_SUGGESTION);
				k = wcslen(TAB_SUGGESTION);
				wordchar_count = k;

				// Print the suggestion and move cursor to the end
				advPrint(line_buffer, CONSOLE_OUTPUT, 1, cursor_orig.Y, POSSIBLE_ATTRIBUTES[DIR_HIGHLIGHT_ATTRIBUTES]);
				moveCursor(1 + k - cursor_loc.X, 0, -1, -1);

				// Lazy stuff to get around an issue
				word_buffer[--wordchar_count] = L'\0';
				highlight_command(word_buffer, ++wordchar_count);
				word_buffer[wordchar_count - 1] = L'\\';
				found = 0;
			}
			break;

		case 13:
			FlushConsoleInputBuffer(CONSOLE_INPUT);
			if (k != 0){
				listening = 0;
			}
			break;

		case L'\b':
			cursor_loc = getCursor();
			// Only backspace if we're within the bounds
			if (cursor_loc.X > 1 || cursor_loc.Y > cursor_orig.Y){

				if (current_dir_tree){
					bst_free(current_dir_tree);
					current_dir_tree = NULL;
					found = 0;
				}

				end_of_line = cursor_loc.X == 0 ? 1 : 0;
				backspace_buffer = line_buffer[k - 1];
				line_buffer[--k] = L'\0';

				if (wordchar_count != 0){
					word_buffer[--wordchar_count] = L'\0';
				}

				// Moving cursor if we've wrapped around
				if (end_of_line){
					cursor_loc.X = width;
					cursor_loc.Y -= 1;
				}
				cursor_loc.X -= 1;

				// Printing spaces over text you want to backspace
				FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width - cursor_loc.X, cursor_loc, &backspace_read);
				FillConsoleOutputAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES], width - cursor_loc.X, cursor_loc, &backspace_read);

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
				clearLine(width * 3, 0, bottom - 2, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);
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

			if (1){
				// Blank any possible usage tips
				clearLine(width * 3, 0, bottom - 2, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

				// Check to see if the command is recognized
				word_buffer[wordchar_count] = L'\0';
				recognized_command = highlight_command(word_buffer, wordchar_count);
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
	clearLine(width, 0, 0, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);
	clearLine(width * 3, 0, bottom - 2, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

	// Move cursor down a line
	cursor_loc = moveCursor(0, 1, 0, -1);

	// Split into array
	line_array = split(line_buffer, L" ", &count);
	*num_words = count;

	free(word_buffer);
	free(line_buffer);
	if (current_dir_tree){
		bst_free(current_dir_tree);
		current_dir_tree = NULL;
	}

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
	SetConsoleMode(CONSOLE_INPUT, (ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT));

	// Style stuff
	CONSOLE_TRANSPARENCY = 240;
	SetWindowLong(ConsoleWindow, GWL_EXSTYLE, GetWindowLong(ConsoleWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(ConsoleWindow, 0, CONSOLE_TRANSPARENCY, LWA_ALPHA);
	HEADER_FOOTER_ATTRIBUTES = 0;
	NORMAL_ATTRIBUTES = 1;
	PROMPT_ATTRIBUTES = 2;
	DIR_HIGHLIGHT_ATTRIBUTES = 3;
	FILE_HIGHLIGHT_ATTRIBUTES = 4;
	TAB_SUGGESTION_ATTRIBUTES = 5;

	// Check for debug flag
	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	// Build command BST from ./commands directory
	command_tree = build_command_tree();

	// Main loop
	while (1) {
		drawPrompt();
		line = readline(&num_words);
		parse(line, num_words);
	}

	free(PATH);
	//free_command_line(line);
	bst_free(command_tree);

	return EXIT_SUCCESS;
}
