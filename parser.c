/*
 * parser.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */
#define _CRT_SECURE_NO_WARNINGS

#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "parser.h"

/* 
* Check to see what the command type is. Like whether it's just a single command or a physical path. 
* This is Windows so it does this by checking if the second character is a semicolon. Will have to be different for Linux.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
int command_type(char *command){
	if (command[1] == ':'){
		return 1;
	}
	return 0;
}

/*
* Converts a normal array of char into a wide char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
wchar_t *convert_to_wchar(char *input){
	// fix this so it's malloc'd dynamically
	wchar_t *command_w[101];
	swprintf(command_w, 100, L"%hs", input);
	return command_w;
}

/* 
* Parses a single command
* Parameters: Command to parse, array of params, type of command)
*/
void parse_command(char *command, char **params, int type) {
	int i = 0;
	char *command_dir = "";
	size_t command_len = strlen(command) + 1;

	// Stuff for CreateProcess
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	// If it's a single command...
	if (type == 0) {
		// Create the path to check the commands subdirectory first
		char *dir = "./commands/";
		size_t dir_len = strlen(dir) + 1;
		command_dir = (char*)malloc(dir_len + command_len);
		strncpy(command_dir, dir, dir_len);
		strncat(command_dir, command, command_len);
	}

	// If it's a full path...
	else if (type == 1) {
		command_dir = command;
	}

	// Spawn the process!
	if (!CreateProcess(convert_to_wchar(command_dir), params[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Error! %d\n", GetLastError());
		// if the error is 2 or 3 (not found) and type is 0 then check in the cwd instead of ./commands/ imo
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

/* 
* Splits a string of space seperated words into an array of words
* Parameter: String to split
* Return: Array of words
*/
char **split(char *str) {
	char *token;
	char **commands = 0;
	char *newline;

	int count = 0;

	token = strtok(str, " ");

	while (token) {
		// Remove newline character
		newline = strchr(token, '\n');
		if (newline) {
			*newline = 0;
		}

		commands = realloc(commands, sizeof(char*)* ++count);
		commands[count - 1] = token;
		token = strtok(0, " ");
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (char*)* (count + 1));
	commands[count] = 0;

	return commands;
}

/* 
* Processes a line of commands
* Parameter: Line to process
*/
void parse(char *cmdline) {
	char **commands;
	char **params;
	int type;
	int i = 0;
	int j = 0;
	int index = 0;

	commands = split(cmdline);
	params = malloc((sizeof(char) * 5) * 20);

	// While there are tokens left...
	while (commands[i]) {

		// If a token isn't a parameter
		if (commands[i][0] != '-'){
			// Get the type
			type = command_type(commands[i]);
			// Check the following tokens for parameters
			j = i + 1;
			index = 0;
			while (commands[j] && commands[j][0] == '-') {
				params[index] = commands[j];
				index++;
				j++;
			}

			// Add null value
			params = realloc(params, sizeof (char*)* (index + 1));
			params[index] = 0;

			// Parse the command and params
			parse_command(commands[i], params, type);
		}

		// Move to next token
		i++;
	}
}
