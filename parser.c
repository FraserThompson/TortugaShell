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
#include <string.h>
#include <Windows.h>
#include "parser.h"
#include "process_mgmt.h"

/* -----CROSS PLATFORM----
* Check to see what the command type is: Whether it's just a single command or a physical path. 
* This is Windows so it does this by checking if the second character is a semicolon. Will have to be different for Linux.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
static int command_type(char *command){
	if (command[1] == ':'){
		return 1;
	}
	return 0;
}

/* -----CROSS PLATFORM----
* Concatenates two strings.
* Parameter: First string, second string.
* Return: Resulting concatenation.
*/
char *concat_string(char *first, char *second){
	size_t first_len = strlen(first) + 1;
	size_t second_len = strlen(second) + 1;
	char *result = (char*)malloc(first_len + second_len);

	if (result == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		return -1;
	}

	strncpy(result, first, first_len);
	strncat(result, second, second_len);

	return result;
}

/* -----CROSS PLATFORM----
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


/* -----CROSS PLATFORM----
* Parses a single command
* Parameters: Command to parse, array of params, type of command)
*/
void parse_command(char *command, char **params, int type) {
	int i = 0;
	int error = 0;
	char *command_dir = command;
	char *command_exe = command;
	char *dir = "./commands/";
	char *exe = ".exe";

	// Processing for single command
	if (type == 0) {
		// Add .exe on the end if not there already
		if (!strstr(command, exe)){
			command_exe = concat_string(command, exe);
		}
		// Add ./commands/ to the beginning so we can check there first
		command_dir = concat_string(dir, command_exe);
	}

	// Spawn the command
	error = create_process_win(command_dir, params);

	if (error == 0) {
		return;
	}

	// If the command isn't found
	if (error == 2 || error == 3){
		// Try again in the CWD
		if (type == 0){
			if (create_process_win(command, params) == 0){
				return;
			}
		}
		printf("'%s' does not exist.\n", command);
	}
}

/* -----CROSS PLATFORM----
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
	params = malloc((sizeof(char) * 5) * 20); //malloc this properly

	// While there are tokens left...
	while (commands[i]) {
		/*// If a token isn't a parameter
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
		i++;*/

		type = command_type(commands[i]);
		parse_command(commands[0], params, type);
	}
}
