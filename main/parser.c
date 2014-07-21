/*
 * parser.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 *
 * Contains methods which help with parsing commands.
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include "cwd.h"
#include "cd.h"
#include "help.h"
#include "parser.h"
#include "shell.h"
#include "myStrings.h"
#include "process_mgmt.h"
#define BUFSIZE 4096

/* -----CROSS PLATFORM----
* Processes a commandline
* Parameter: Line to process
*/
void parse(wchar_t *cmdline) {
	wchar_t **full_line;
	int last_index = 0;
	int i = 0;
	int error;

	// Initialize the struct
	command_line *line = emalloc(sizeof(command_line));
	line->params = malloc(sizeof(wchar_t));
	line->params = L"";
	line->type = 0;
	line->command = NULL;
	line->pipe = NULL;
	line->redirectIn = NULL;
	line->redirectOut = NULL;

	// Split the raw line into tokens for processing
	if (debug_global){ wprintf(L"PARSE: Input: %s\n", cmdline); }
	full_line = split(cmdline, L" ", &last_index);
	if (debug_global > 1){ wprintf(L"PARSE: First item: %s\n", full_line[0]); }

	// First token will always be a command so add it to the struct and check type
	line->command = emalloc(wcslen(full_line[i] + 1) * sizeof(wchar_t));
	wcscpy(line->command, full_line[i]);
	line->type = get_command_type(full_line[i]);

	// Checking for internal commands 
	if (wcscmp(line->command, L"cwd") == 0){
		if (debug_global){ wprintf(L"PARSE: Got cwd.\n"); }
		line->params = concat_string(line->params, L" ", getCWD());
		line->command = L"echo";
		line->type = 0;
	}
	else if (wcscmp(line->command, L"help") == 0){
		if (debug_global){ wprintf(L"PARSE: Got help.\n"); }
		print_help();
		return;
	}

	/* If there's more than one token */
	if (last_index > 1){
		/* Internal commands with params*/
		if (wcscmp(line->command, L"cd") == 0){
			if (debug_global){ wprintf(L"PARSE: Got cd, changing directory.\n"); }
			cd(full_line[1]);
			if (debug_global){ wprintf(L"PARSE: Done with cd, returning.\n"); }
			return;
		}

		i++; // increment i to skip the first token since we've already delt with it

		/* Process line and populate struct */
		while (full_line[i]){
			if (debug_global > 1) wprintf(L"PARSE: Working on item: %ws\n", full_line[i]);

			if (wcscmp(full_line[i], L">") == 0){
				if (debug_global) wprintf(L"PARSE: Adding redirectOut location: %s\n", full_line[i + 1]);
				line->redirectOut = emalloc(wcslen(full_line[++i]) * sizeof(wchar_t));
				wcscpy(line->redirectOut, full_line[i]);
			}
			else if (wcscmp(full_line[i], L"<") == 0){
				if (debug_global) wprintf(L"PARSE: Adding redirectIn location: %s\n", full_line[i + 1]);
				line->redirectIn = emalloc(wcslen(full_line[++i]) * sizeof(wchar_t));
				wcscpy(line->redirectIn, full_line[i]);
			}
			else {
				if (debug_global) wprintf(L"PARSE: Adding parameter %s\n", full_line[i]);
				line->params = concat_string(line->params, L" ", full_line[i]);
			}
			i++;
		}
	}
	line->params = wcscmp(line->params, L"") != 0 ? line->params : NULL; //Set params to NULL if empty

	if (debug_global) wprintf(L"PARSE: Sending the following to create_process for execution:\n", line->command);
	if (debug_global) display_info(line);

	error = create_process(line);

	if (error == 2 || error == 3){
		fwprintf(stderr, L"PARSE: '%s' does not exist.\n", line->command);
	}

	if (error == 50){
		fwprintf(stderr, L"PARSE: Redirection error.\n");
	}

	// Free the stuff we don't need any more
	free(cmdline);
	//free_stuff(full_line, last_index);

}

static void free_stuff(wchar_t **full_line, int last_index){
	int i = 0;
	while (i < last_index - 1){
		wprintf(L"Freeing: %s", full_line[i]);
		free(full_line[i]);
		i++;
	}
}

/* -------CROSS-PLATFORM------
* Prints the info contained in the command_line struct
* Param: command_line struct
*/
void display_info(command_line *line){
	int i = 0;
	if (debug_global > 1) wprintf(L"DISPLAY_INFO: Displaying info contained in line struct\n"); 
	wprintf(L"%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%-5s\n", L"Command", L"Argv", L"redirectOut", L"redirectIn", L"pipe", L"type");
	wprintf(L"%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%i\n", line->command, line->params, line->redirectOut, line->redirectIn, line->pipe, line->type);
}

/* -------WINDOWS------
* Returns the path to the system dir.
* Return: path to the system dir
*/
wchar_t *get_system_dir(void){
	size_t size = 100;
	wchar_t buffer[BUFSIZE];
	if (debug_global > 1){ wprintf(L"GET_SYSTEM_DIR: Getting system dir...\n"); }
	if (!GetSystemDirectory(buffer, size)){
		wprintf(L"GET_SYSTEM_DIR: Error getting system dir!\n");
		exit(EXIT_FAILURE);
	}
	return concat_string(buffer, L"\\", NULL);
}

/* -------WINDOWS------
* Returns the PATH the application was run from with \\commands\\ on the end.
* Return: path
*/
wchar_t  *get_commands_dir(void){
	if (debug_global > 1) wprintf(L"GET_COMMANDS_DIR: Getting commands dir...\n"); 
	return concat_string(PATH, L"\\commands\\", NULL);
}

/* -------WINDOWS------
* Returns the command with the extension added (.exe in Windows).
* Parameter: Command to attach it to.
* Return: Command with extension added
*/
wchar_t  *get_command_ext(wchar_t *command){
	if (debug_global > 1) wprintf(L"GET_COMMANDS_EXT: Adding extension to command...\n");
	return concat_string(command, L".exe", NULL);
}

/* -----WINDOWS----
* Check to see what the command type is: Whether it's just a single command or a physical path.
* This is Windows so it does this by checking if the second character is a semicolon. Will have to be different for Linux.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
int get_command_type(wchar_t  *command){
	if (debug_global){ wprintf(L"GET_COMMAND_TYPE: Input: %s\n", command); }
	if (command[1] == L':'){
		if (debug_global){ wprintf(L"GET_COMMAND_TYPE: It's a path.\n"); }
		return 1;
	}
	if (debug_global){ wprintf(L"GET_COMMAND_TYPE: It's a command.\n"); }
	return 0;
}