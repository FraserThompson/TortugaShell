/*
* shell.c
*
*  Created on: 29/04/2014
*      Author: Fraser
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 300
#define MAX_WORD 64
#define NUM_COMMANDS 5
#define NUM_ATTRIBUTES 11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include "process_mgmt.h"
#include "parser.h"
#include "myStrings.h"
#include "shell.h"
#include "gui.h"
#include "cwd.h"
#include "console.h"

int debug_global = 0;
int play_song = 0;
int found = 0; // used as a flag for highlight_command to denote whether we need to build another directory bst
int CONSOLE_TRANSPARENCY = 240;
wchar_t *PATH;
HANDLE CONSOLE_OUTPUT;
HANDLE CONSOLE_INPUT;
int HEADER_FOOTER_ATTRIBUTES = 0;
int NORMAL_ATTRIBUTES = 1;
int PROMPT_ATTRIBUTES = 2;
int DIR_HIGHLIGHT_ATTRIBUTES = 3;
int FILE_HIGHLIGHT_ATTRIBUTES = 4;
int TAB_SUGGESTION_ATTRIBUTES = 5;
WORD POSSIBLE_ATTRIBUTES[NUM_ATTRIBUTES] = { (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY), (FOREGROUND_INTENSITY), (FOREGROUND_RED), 
(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY),(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY), (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED), 
(FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE), (FOREGROUND_BLUE | FOREGROUND_INTENSITY), (FOREGROUND_GREEN | FOREGROUND_INTENSITY), (FOREGROUND_RED | FOREGROUND_GREEN),
(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)};
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
	if (line->output != NULL){
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
				newnode->title = wcsncat(newnode->title, L"\\", 2);
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
	wchar_t *recognized_commands[NUM_COMMANDS] = { L"cwd", L"help", L"cd", L"settings", L"quit"};
	wchar_t *command_usage[NUM_COMMANDS] = { L"cwd\tPrints the current working directory.\n\tUsage: cwd [directory] [-h]\n", L"help\tPrints a list of possible commands.\n\tUsage: help\n",
		L"cd\tChanges the current working directory.\n\tUsage: cd [directory] [-h]\n", L"settings\tDisplays a window which lets you adjust settings.\n\t\tUsage: settings\n", 
		L"quit\tQuits Tortuga.\n\tUsage: quit\n"};

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

/* -----CROSS-PLATFORM----
* Updates style.txt with current values
*/
static void write_style_file(){
	wchar_t *file_path = concat_string(PATH, L"\\style.txt", NULL);
	FILE *style_f = _wfopen(file_path, L"w");
	fprintf(style_f, "%d %d %d %d %d %d %d %d %d", HEADER_FOOTER_ATTRIBUTES, NORMAL_ATTRIBUTES, PROMPT_ATTRIBUTES, DIR_HIGHLIGHT_ATTRIBUTES, FILE_HIGHLIGHT_ATTRIBUTES, TAB_SUGGESTION_ATTRIBUTES, CONSOLE_TRANSPARENCY, debug_global, play_song);
	fclose(style_f);
}


/* -----WINDOWS----
* Clears a space at the top and prints the wchar content string in the middle of it.
* Params: Wchar string to print, handle of CONSOLE_OUTPUT
*/
static void printHeader(wchar_t *content){
	int width = getConsoleWidth(CONSOLE_OUTPUT);
	int len;
	int center_x;
	COORD topCoords;

	// Work out where to put stuff
	len = wcslen(content);
	center_x = width / 2 - len / 2;
	topCoords.Y = getConsoleTop(CONSOLE_OUTPUT);
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
	int width = getConsoleWidth(CONSOLE_OUTPUT);

	// Work out where to put stuff
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	bottomCoords.X = 0;
	bottomCoords.Y = getConsoleBottom(CONSOLE_OUTPUT) - 2;

	clearLine(width, bottomCoords.X, bottomCoords.Y, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

	// Print the text
	if (content != NULL){
		clearLine(width, bottomCoords.X, bottomCoords.Y, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);
		clearLine(width, bottomCoords.X, bottomCoords.Y + 1, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);
		advPrint(content, CONSOLE_OUTPUT, 0, bottomCoords.Y, POSSIBLE_ATTRIBUTES[HEADER_FOOTER_ATTRIBUTES]);
	}

	SetConsoleTextAttribute(CONSOLE_OUTPUT, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);
}

/* -----WINDOWS----
* Displays the style settings menu which is accessed from the main settings menu
*/
static void style_settings(){
	COORD current_cursor = getCursor(CONSOLE_OUTPUT);
	COORD options_cursor = { 1, 1 };
	COORD cursor_cursor = { 1, 1 };
	COORD original_cursor = current_cursor;
	WORD select_attributes = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;

	int num_options = 7;
	wchar_t *intstr = emalloc(sizeof(wchar_t)* 20);
	swprintf(intstr, 20, L"TRANSPARENCY: %d  ", CONSOLE_TRANSPARENCY);
	wchar_t *options[7] = { L"HEADER FOOTER", L"NORMAL", L"PROMPT", L"DIR HIGHLIGHT", L"FILE HIGHLIGHT", L"TAB SUGGESTION", intstr };
	int attributes[7] = { HEADER_FOOTER_ATTRIBUTES, NORMAL_ATTRIBUTES, PROMPT_ATTRIBUTES, DIR_HIGHLIGHT_ATTRIBUTES, FILE_HIGHLIGHT_ATTRIBUTES, TAB_SUGGESTION_ATTRIBUTES, 10 };

	int i = 0;
	int height = 10;
	int width = 30;
	int listening = 1;
	int offsetY = 4;
	int offsetX = 9;

	wint_t input;
	wint_t secondInput;

	options_cursor = draw_settings(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
		L"STYLES", L"PRESS ENTER TO APPLY", options, attributes, num_options, height, width, 2, offsetX, offsetY);
	current_cursor = getCursor(CONSOLE_OUTPUT);
	cursor_cursor = current_cursor;

	while (listening){
		// Draw the selection cursors;
		advPrint(L"<", CONSOLE_OUTPUT, cursor_cursor.X, current_cursor.Y, select_attributes);
		advPrint(L">", CONSOLE_OUTPUT, cursor_cursor.X + width - 2, current_cursor.Y, select_attributes);
		input = _getwch();
		switch (input){
		case 224:
		case 0:
			secondInput = _getwch();
			// Down
			if (secondInput == 80){
				if (i < num_options - 1){
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
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
					current_cursor = moveCursor(0, -1, -1, -1, CONSOLE_OUTPUT);
					advPrint(L">", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
					advPrint(L"<", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
					i--;

				}
			}
			// Left
			else if (secondInput == 75){
				current_cursor = moveCursor(0, 0, cursor_cursor.X, current_cursor.Y, CONSOLE_OUTPUT);
				switch (i){
				case 0:
					if (HEADER_FOOTER_ATTRIBUTES > 0) HEADER_FOOTER_ATTRIBUTES--;
					attributes[0] = HEADER_FOOTER_ATTRIBUTES;
					break;
				case 1:
					if (NORMAL_ATTRIBUTES > 0) NORMAL_ATTRIBUTES--;
					attributes[1] = NORMAL_ATTRIBUTES;
					break;
				case 2:
					if (PROMPT_ATTRIBUTES > 0) PROMPT_ATTRIBUTES--;
					attributes[2] = PROMPT_ATTRIBUTES;
					break;
				case 3:
					if (DIR_HIGHLIGHT_ATTRIBUTES > 0) DIR_HIGHLIGHT_ATTRIBUTES--;
					attributes[3] = DIR_HIGHLIGHT_ATTRIBUTES;
					break;
				case 4:
					if (FILE_HIGHLIGHT_ATTRIBUTES > 0) FILE_HIGHLIGHT_ATTRIBUTES--;
					attributes[4] = FILE_HIGHLIGHT_ATTRIBUTES;
					break;
				case 5:
					if (TAB_SUGGESTION_ATTRIBUTES > 0) TAB_SUGGESTION_ATTRIBUTES--;
					attributes[5] = TAB_SUGGESTION_ATTRIBUTES;
					break;
				case 6:
					if (CONSOLE_TRANSPARENCY > 50) {
						CONSOLE_TRANSPARENCY--;
						swprintf(intstr, 20, L"TRANSPARENCY: %d  ", CONSOLE_TRANSPARENCY);
						wcscpy(options[6], intstr);
						setTransparency(CONSOLE_TRANSPARENCY);
					}
					break;
				}
				original_cursor = getCursor(CONSOLE_OUTPUT);
				current_cursor = moveCursor(0, 0, options_cursor.X, options_cursor.Y, CONSOLE_OUTPUT);
				current_cursor = draw_options(options, attributes, num_options, width, current_cursor, offsetX, offsetY);
				current_cursor = moveCursor(0, 0, original_cursor.X, original_cursor.Y, CONSOLE_OUTPUT);
			}
			// Right
			else if (secondInput == 77){
				current_cursor = moveCursor(0, 0, cursor_cursor.X + width - 2, current_cursor.Y, CONSOLE_OUTPUT);
				switch (i){
				case 0:
					if (HEADER_FOOTER_ATTRIBUTES < NUM_ATTRIBUTES - 1) HEADER_FOOTER_ATTRIBUTES++;
					attributes[0] = HEADER_FOOTER_ATTRIBUTES;
					break;
				case 1:
					if (NORMAL_ATTRIBUTES < NUM_ATTRIBUTES - 1) NORMAL_ATTRIBUTES++;
					attributes[1] = NORMAL_ATTRIBUTES;
					break;
				case 2:
					if (PROMPT_ATTRIBUTES < NUM_ATTRIBUTES - 1) PROMPT_ATTRIBUTES++;
					attributes[2] = PROMPT_ATTRIBUTES;
					break;
				case 3:
					if (DIR_HIGHLIGHT_ATTRIBUTES < NUM_ATTRIBUTES - 1) DIR_HIGHLIGHT_ATTRIBUTES++;
					attributes[3] = DIR_HIGHLIGHT_ATTRIBUTES;
					break;
				case 4:
					if (FILE_HIGHLIGHT_ATTRIBUTES < NUM_ATTRIBUTES - 1) FILE_HIGHLIGHT_ATTRIBUTES++;
					attributes[4] = FILE_HIGHLIGHT_ATTRIBUTES;
					break;
				case 5:
					if (TAB_SUGGESTION_ATTRIBUTES < NUM_ATTRIBUTES - 1) TAB_SUGGESTION_ATTRIBUTES++;
					attributes[5] = TAB_SUGGESTION_ATTRIBUTES;
					break;
				case 6:
					if (CONSOLE_TRANSPARENCY < 254) {
						CONSOLE_TRANSPARENCY++;
						swprintf(intstr, 20, L"TRANSPARENCY: %d  ", CONSOLE_TRANSPARENCY);
						wcscpy(options[6], intstr);
						setTransparency(CONSOLE_TRANSPARENCY);
					}
					break;
				}
				original_cursor = getCursor(CONSOLE_OUTPUT);
				current_cursor = moveCursor(0, 0, options_cursor.X, options_cursor.Y, CONSOLE_OUTPUT);
				current_cursor = draw_options(options, attributes, num_options, width, current_cursor, offsetX, offsetY);
				current_cursor = moveCursor(0, 0, original_cursor.X, original_cursor.Y, CONSOLE_OUTPUT);
			}
			break;

		case L'\r':
			listening = 0;
			break;
		}
	}

	write_style_file();
	current_cursor = clear_a_box(current_cursor, height, width, offsetX, offsetY);
	free(intstr);
}

/* -----WINDOWS----
* Displays the main settings menu.
* Return: 0 if we're to quit, 1 if we should continue displaying
*/
int main_settings(){
	COORD current_cursor = getCursor(CONSOLE_OUTPUT);
	COORD options_cursor = { 1, 1 };
	WORD select_attributes = BACKGROUND_BLUE | FOREGROUND_INTENSITY;
	WORD background_attributes = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
	WORD border_attributes = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
	wchar_t *title = L"SETTINGS";
	wchar_t *footer = L"SELECT AN OPTION";

	int numChars; //stores value for fillconsoleoutputattribute
	int num_options = 4;
	wchar_t *options[4] = { L"STYLE", debug_global ? L"DEBUG ON" : L"DEBUG OFF", play_song ? L"STARTUP SONG ON" : L"STARTUP SONG OFF", L"EXIT" };
	int attributes[4] = { 10, 10, 10, 10 };

	int i = 0;
	int exit;
	int listening = 1;
	int width = 22;
	int height = 7;
	int offsetX = 0;
	int offsetY = 0;

	wint_t input;
	wint_t secondInput;

	current_cursor = draw_settings(border_attributes, background_attributes, title, footer, options, attributes, num_options, height, width, 5, offsetX, offsetY);
	current_cursor = getCursor(CONSOLE_OUTPUT);

	while (listening){
		// Draw the selection cursor
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, select_attributes, width - 1, current_cursor, &numChars);
		input = _getch();
		switch (input){
		case 224:
		case 0:
			secondInput = _getch();
			// Down
			if (secondInput == 80){
				if (i < num_options - 1){
					//blank old cursor
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, background_attributes, width - 1, current_cursor, &numChars);
					//move down
					current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
					//draw cursor
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, select_attributes, width - 1, current_cursor, &numChars);
					i++;
				}
			}
			// Up
			else if (secondInput == 72){
				if (i > 0){
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, background_attributes, width - 1, current_cursor, &numChars);
					current_cursor = moveCursor(0, -1, -1, -1, CONSOLE_OUTPUT);
					FillConsoleOutputAttribute(CONSOLE_OUTPUT, select_attributes, width - 1, current_cursor, &numChars);
					i--;

				}
			}
			// Left
			else if (secondInput == 75){
				switch (i){
				case 0:
					break;
				case 1:
					break;
				case 2:
					break;
				}
			}
			// Right
			else if (secondInput == 77){
				switch (i){
				case 0:
					break;
				case 1:
					break;
				case 2:
					break;
				}
			}
			break;

					case L'\r':
						switch (i){
						case 0:
							listening = 0;
							break;
						case 1:
							listening = 0;
							break;
						case 2:
							listening = 0;
							break;
						case 3:
							listening = 0;
							break;
						}
						break;
					}
				}

	if (i == 0){
		current_cursor = draw_a_box(current_cursor, BACKGROUND_GREEN, BACKGROUND_GREEN, title, footer, height, width, 0, 0, 0);
		style_settings();
		exit = 1;
	}
	else if (i == 1){
		if (debug_global){
			debug_global = 0;
		}
		else{
			debug_global = 1;
		}

		exit = 1;
	}
	else if (i == 2){
		if (play_song){
			play_song = 0;
		}
		else{
			play_song = 1;
		}

		exit = 1;
	}
	else if (i == 3){
		clear_a_box(current_cursor, height, width, offsetX, offsetY);
		exit = 0;
	}

	write_style_file();
	return exit;
}



/* -----WINDOWS----
* Draws a prompt at the top containing the cwd
*/
static void drawPrompt(void) {
	wchar_t *top = concat_string(L"", getCWD(), L"\n");
	COORD current_cursor = getCursor(CONSOLE_OUTPUT);
	int height = getConsoleHeight(CONSOLE_OUTPUT);
	int width = getConsoleWidth(CONSOLE_OUTPUT);

	// If we're going to be printing over the footer then move everything down
	if (current_cursor.Y >= height - 4){

		//Clear footer
		clearLine(width, 0, height, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

		//Clear header
		clearLine(width, 0, 0, POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES]);

		// Move cursor down to scroll then back up for typing
		current_cursor = moveCursor(0, 4, -1, -1, CONSOLE_OUTPUT);
		current_cursor = moveCursor(0, -5, -1, -1, CONSOLE_OUTPUT);
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
static int highlight_command(wchar_t *command, int wordchar_count, int dirs_only){
	COORD cursor_loc = getCursor(CONSOLE_OUTPUT); //current cursor location
	COORD word_begin = cursor_loc; //cursor location at the beginning of the word
	word_begin.X -= wordchar_count;

	//bst stuff
	node *parent;
	node *other_parent;
	node *result = NULL;
	node *other_result = NULL;

	DWORD num_read;
	DWORD written;

	int width = getConsoleWidth(CONSOLE_OUTPUT);
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
	if (command_tree != NULL && dirs_only == 0){
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
	wchar_t backspace_buffer;
	wint_t wcs_buffer;
	wint_t second_wcs;
	DWORD backspace_read;
	COORD cursor_loc;
	COORD word_begin = getCursor(CONSOLE_OUTPUT);
	COORD cursor_orig = getCursor(CONSOLE_OUTPUT); //location of the cursor before anything has happened
	int tab_suggestion_len;
	int count;
	int end_of_line = 0;
	int width = getConsoleWidth(CONSOLE_OUTPUT);
	int bottom = getConsoleBottom(CONSOLE_OUTPUT);
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

		case 224:
		case 0:
			second_wcs = _getwch();
			switch (second_wcs){
			case 77:
				cursor_loc = getCursor(CONSOLE_OUTPUT);
				if (cursor_loc.X < k + 1){
					moveCursor(1, 0, -1, -1, CONSOLE_OUTPUT);
				}
				break;
			case 75:
				cursor_loc = getCursor(CONSOLE_OUTPUT);
				if (cursor_loc.X > 1){
					moveCursor(-1, 0, -1, -1, CONSOLE_OUTPUT);
				}
				break;
			}

		case 27:
			break;

		case L'\t':
			if (TAB_SUGGESTION){
				cursor_loc = getCursor(CONSOLE_OUTPUT);
				tab_suggestion_len = wcslen(TAB_SUGGESTION);

				// Remove the word fragment from the line buffer
				k -= wordchar_count;
				cursor_loc.X -= wordchar_count;
				line_buffer[k] = L'\0';

				// Add the tab suggestion to the line buffer
				wcsncat(line_buffer, TAB_SUGGESTION, tab_suggestion_len + 1);
				k += tab_suggestion_len;

				//Add the tab suggestion to the wordbuffer
				wcsncpy(word_buffer, TAB_SUGGESTION, tab_suggestion_len + 1);
				wordchar_count = tab_suggestion_len;

				// Print the suggestion and move cursor to the end
				advPrint(TAB_SUGGESTION, CONSOLE_OUTPUT, cursor_loc.X, cursor_orig.Y, POSSIBLE_ATTRIBUTES[DIR_HIGHLIGHT_ATTRIBUTES]);
				moveCursor(0, 0, k + 1, -1, CONSOLE_OUTPUT);

				// Stuff to make it generate the BST for a new directory
				word_buffer[--wordchar_count] = L'\0';
				highlight_command(word_buffer, ++wordchar_count, 1);
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
			cursor_loc = getCursor(CONSOLE_OUTPUT);
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
				cursor_loc = moveCursor(1, 0, -1, -1, CONSOLE_OUTPUT);
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
			word_begin = getCursor(CONSOLE_OUTPUT);
			putwchar(wcs_buffer);
			break;

		default:

			cursor_loc = getCursor(CONSOLE_OUTPUT);

			// If the cursor is at the same position in the line array
			if (cursor_loc.X == k + 1){
				line_buffer[k++] = wcs_buffer;
				word_buffer[wordchar_count++] = wcs_buffer;
			} 
			// If the cursor is less than the position in the line array
			else if(cursor_loc.X < k + 1){
				line_buffer[cursor_loc.X - 1] = wcs_buffer;
			}
			putwchar(wcs_buffer);

			// Check to see if the command is recognized
			word_buffer[wordchar_count] = L'\0';
			recognized_command = highlight_command(word_buffer, wordchar_count, word_count);

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
	cursor_loc = moveCursor(0, 1, 0, -1, CONSOLE_OUTPUT);

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

/* -----WINDOWS----
* Plays a small song comprised of five (sort of) random notes of (sort of) random duration
*/
void random_song(){
	int highest_freq = 1500;
	int lowest_freq = 80;
	int longest_dur = 200;
	int shortest_dur = 50;
	int r = rand() % (longest_dur + 1 - shortest_dur) + shortest_dur;
	int g = rand() % (highest_freq + 1 - lowest_freq) + lowest_freq;
	for (int i = 0; i < 5; i++){
		Beep(g, r);
		r = rand() % (longest_dur + 1 - shortest_dur) + shortest_dur;
		g = rand() % (highest_freq + 1 - lowest_freq) + lowest_freq;
	}
}

/* 
* Main loop. Reads a line and parses it.
*/
int wmain(int argc, wchar_t *argv[]) {
	int i = 0;
	int exit = 0;
	int num_words;
	wchar_t *cwd = getCWD();
	wchar_t **line = NULL;
	HWND ConsoleWindow;
	FILE *style_f;

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

	// Restoring style data from style.txt
	style_f = fopen("style.txt", "r");
	if (NULL == style_f){
		style_f = fopen("style.txt", "w");
		fprintf(style_f, "%d %d %d %d %d %d %d %d %d", HEADER_FOOTER_ATTRIBUTES, NORMAL_ATTRIBUTES, PROMPT_ATTRIBUTES, DIR_HIGHLIGHT_ATTRIBUTES, FILE_HIGHLIGHT_ATTRIBUTES, TAB_SUGGESTION_ATTRIBUTES, CONSOLE_TRANSPARENCY, debug_global, play_song);
	}
	else {
		fscanf(style_f, "%d %d %d %d %d %d %d %d %d", &HEADER_FOOTER_ATTRIBUTES, &NORMAL_ATTRIBUTES, &PROMPT_ATTRIBUTES, &DIR_HIGHLIGHT_ATTRIBUTES, &FILE_HIGHLIGHT_ATTRIBUTES, &TAB_SUGGESTION_ATTRIBUTES, &CONSOLE_TRANSPARENCY, &debug_global, &play_song);
	}
	fclose(style_f);
	setTransparency(CONSOLE_TRANSPARENCY);
	srand(time(NULL));
	// Check for debug flag
	while (argv[i]){
		if ((wcscmp(argv[i], L"-d") == 0) || (wcscmp(argv[i], L"-debug") == 0)) {
			debug_global = 1;
		}
		i++;
	}

	// Build command BST from ./commands directory
	command_tree = build_command_tree();
	if (play_song){
		random_song();
	}

	// Main loop
	while (!exit) {
		drawPrompt();
		line = readline(&num_words);
		exit = parse(line, num_words);
		free_word_array(line, num_words);
	}

	free(PATH);
	bst_free(command_tree);
	return EXIT_SUCCESS;
}
