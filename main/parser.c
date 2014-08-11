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
#include "console.h"
#include "myStrings.h"
#include "process_mgmt.h"
#define BUFSIZE 4096

/* -----CROSS PLATFORM----
* Processes a commandline
* Parameter: Line to process
*/
int parse(wchar_t **cmdline, int num_words) {
	int last_index = num_words;
	int i = 0;
	int error;


	if (num_words < 1){
		fwprintf(stderr, L"PARSE: Error parsing tokens. Did you remember to close your quotation marks?\n");
		return 0;
	}


	// Initialize the struct
	command_line *line = init_command_line(NULL, L"", NULL, NULL, NULL, 0);

	if (debug_global > 1){ wprintf(L"PARSE: Dealing with %i items. First item: %s\n", last_index, cmdline[0]); }

	// First token will always be a command so add it to the struct and check type
	line->command = _wcsdup(cmdline[i]);
	line->type = get_command_type(cmdline[i]);

	// Checking for internal commands 
	if (wcscmp(line->command, L"cwd") == 0){
		if (debug_global){ wprintf(L"PARSE: Got cwd.\n"); }
		line->params = concat_string(line->params, L" ", getCWD());
		free(line->command);
		line->command = _wcsdup(L"echo");
		line->type = 0;
	}
	else if (wcscmp(line->command, L"help") == 0){
		if (debug_global){ wprintf(L"PARSE: Got help.\n"); }
		/*line->params = get_help();
		line->command = _wcsdup(L"echo");
		line->type = 0;*/
		get_help();
		free_command_line(line);
		return 0;
	}
	else if (wcscmp(line->command, L"settings") == 0){
		if (debug_global){ wprintf(L"PARSE: Got settings.\n"); }
		int t = 1;
		clearScreen(CONSOLE_OUTPUT);
		while (t){
			t = main_settings();
		}
		free_command_line(line);
		clearScreen(CONSOLE_OUTPUT);
		moveCursor(0, 0, 1, 0, CONSOLE_OUTPUT);
		return 0;
	}
	else if (wcscmp(line->command, L"quit") == 0){
		if (debug_global){ wprintf(L"PARSE: Got quit.\n"); }
		free_command_line(line);
		return 1;
	}
	else if (wcscmp(line->command, L"sing") == 0){
		if (debug_global){ wprintf(L"PARSE: Got sing.\n"); }
		random_song();
		return 0;
	}

	/* If there's more than one token */
	if (last_index > 1){
		/* Internal commands with params*/
		if (wcscmp(line->command, L"cd") == 0){
			if (debug_global){ wprintf(L"PARSE: Got cd, changing directory to %s.\n", cmdline[1]); }
			cd(cmdline[1]);
			if (debug_global){ wprintf(L"PARSE: Done with cd, returning.\n"); }
			free_command_line(line);
			return 0;
		}

		i++; // increment i to skip the first token since we've already delt with it

		/* Process line and populate struct */
		while (cmdline[i]){
			if (debug_global) wprintf(L"PARSE: Working on item: %ws\n", cmdline[i]);

			if (wcscmp(cmdline[i], L">") == 0){
				i++;
				if (debug_global) wprintf(L"PARSE: Adding redirectOut location: %s\n", cmdline[i]);
				line->redirectOut = _wcsdup(cmdline[i]);
			}
			else if (wcscmp(cmdline[i], L"<") == 0){
				i++;
				if (debug_global) wprintf(L"PARSE: Adding redirectIn location: %s\n", cmdline[i]);
				line->redirectOut = _wcsdup(cmdline[i]);
			}
			else {
				if (debug_global) wprintf(L"PARSE: Adding parameter %s\n", cmdline[i]);
				line->params = concat_string(line->params, L" ", cmdline[i]);
			}
			i++;
		}
	}
	line->params = wcscmp(line->params, L"") != 0 ? line->params : NULL; //Set params to NULL if empty

	if (debug_global) wprintf(L"PARSE: Sending the following to create_process for execution: %s\n", line->command);
	if (debug_global) display_info(line);

	error = create_process(line);

	if (error == 2 || error == 3 ){
		fwprintf(stderr, L"PARSE: '%s' is not recognized as a path to a file or an internal/external command.\n", line->command);
	}

	if (error == 5){
		fwprintf(stderr, L"PARSE: '%s' is a directory.\n", line->command);
	}
	if (error == 50){
		fwprintf(stderr, L"PARSE: Redirection error.\n");
	}

	free_command_line(line);
	return 0;
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
	wchar_t buffer[BUFSIZE];
	if (debug_global > 1){ wprintf(L"GET_SYSTEM_DIR: Getting system dir...\n"); }
	if (!GetSystemDirectory(buffer, BUFSIZE)){
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