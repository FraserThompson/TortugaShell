/*
 * parser.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 *
 * Contains methods which help with parsing commands.
 */
#define _CRT_SECURE_NO_WARNINGS
#define NUM_DIRS 3

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cwd.h"
#include "cd.h"
#include "help.h"
#include "parser.h"
#include "process_mgmt.h"

/* -----CROSS PLATFORM----
* Check to see what the command type is: Whether it's just a single command or a physical path. 
* This is Windows so it does this by checking if the second character is a semicolon. Will have to be different for Linux.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
static int command_type(char *command){
	if (debug_global){ printf("COMMAND_TYPE: Input: %s\n", command); }
	if (command[1] == ':'){
		if (debug_global){ printf("COMMAND_TYPE: It's a path.\n"); }
		return 1;
	}
	if (debug_global){ printf("COMMAND_TYPE: It's a command.\n"); }
	return 0;
}

/* -----CROSS PLATFORM----
* Concatenates up to three strings.
* Parameter: First string, second string, third string (or null).
* Return: Resulting concatenation.
*/
char *concat_string(char *first, char *second, char *third){
	size_t first_len = strlen(first) + 1;
	size_t second_len = strlen(second) + 1;
	size_t third_len = 0;

	if (debug_global){ printf("CONCAT_STRING: Input: %s %s\n", first, second); }
	if (third){
		if (debug_global){ printf("CONCAT_STRING: Input: %s\n", third); }
		third_len = strlen(third) + 1;
	}

	char *result = (char*)malloc(first_len + second_len + third_len);

	if (result == NULL){
		fprintf(stderr, "CONCAT_STRING: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if (debug_global){ printf("CONCAT_STRING: Adding first: %s\n", first); }
	strncpy(result, first, first_len);
	if (debug_global){ printf("CONCAT_STRING: Adding second: %s\n", second); }
	strncat(result, second, second_len);

	if (third){
		if (debug_global) { printf("CONCAT_STRING: Adding third: %s\n", third);  }
		strncat(result, third, third_len);
	}

	if (debug_global) { printf("CONCAT_STRING: Returning: %s\n", result); }
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

	if (debug_global){ printf("SPLIT: Input: %s\n", str); }

	while (token) {
		if (debug_global){ printf("SPLIT: Working on token: %s\n", token); }

		// Remove newline character
		newline = strchr(token, '\n');
		if (newline) {
			*newline = 0;
		}

		commands = realloc(commands, sizeof(char*)* ++count);
		if (commands == NULL){
			printf("SPLIT: Error during realloc!\n");
			exit(EXIT_FAILURE);
		}
		commands[count - 1] = token;
		token = strtok(0, " ");
		if (debug_global){ printf("SPLIT: Done with token: %s\n", commands[count - 1]); }
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (char*)* (count + 1));

	if (commands == NULL){
		fprintf(stderr, "SPLIT: Error during realloc!\n");
		exit(EXIT_FAILURE);
	}

	commands[count] = 0;
	if (debug_global){ printf("SPLIT: Returning\n"); }
	return commands;
}


/* -----CROSS PLATFORM----
* Parses a single command
* Parameters: Command to parse, parameters string, type of command)
*/
void parse_command(char *command, char *params, int type) {
	int error = 0;
	int i = 0;
	char *command_dir = command;
	char *command_exe = command;
	char *system_dir = concat_string(get_system_dir(), "\\", NULL);
	char *dirs[NUM_DIRS] = { PATH, system_dir, "./"};
	char *exe = ".exe"; 

	if (debug_global){ printf("PARSE_COMMAND: Input: %s %s %i\n", command, params, type); }

	// Processing a relative path
	if (type == 0) {

		// Add .exe on the end if not there already
		if (!strstr(command, exe)){
			command_exe = concat_string(command, exe, NULL);
		}

		// Check for the desired command in all dirs until found
		while (i != NUM_DIRS){
			command_dir = concat_string(dirs[i], command_exe, NULL);
			i++;
			if (debug_global){ printf("PARSE_COMMAND: Trying to create %s as a process with params %s\n", command_dir, params); }
			if (params){
				params = concat_string(command_dir, " ", params);
			}
			error = create_process(command_dir, params);
			if (error == 0) {
				return;
			}
			else {
				if (debug_global){ printf("PARSE_COMMAND: Unable to create process error %i\n", error); }
			}
		}

	}

	// Processing a full path
	if (type == 1){
		error = create_process(command, params);
		if (params){
			params = concat_string(command, " ", params);
		}
		if (error == 0) {
			return;
		}
	}

	printf("'%s' does not exist.\n", command);
}

/* -----CROSS PLATFORM----
* Processes a commandline
* Parameter: Line to process
*/
void parse(char *cmdline) {
	char **commands;
	char *params = NULL;
	int type;
	int i = 2;

	if (debug_global){ printf("PARSE: Input: %s\n", cmdline); }
	commands = split(cmdline);
	if (debug_global){ printf("PARSE: First command: %s\n", commands[0]); }

	// If the user is trying to cd
	if (strcmp(commands[0], "cd") == 0){
		cd(commands[1]);
		return;
	}
	// If the user is trying to get cwd
	else if (strcmp(commands[0], "cwd") == 0){
		printf("%s\n", getCWD(commands[1]));
		return;
	}
	// If the user is trying to get to help
	else if (strcmp(commands[0], "help") == 0){
		print_help();
		return;
	}

	type = command_type(commands[0]);

	// If there's more than just one thing
	if (commands[1]){
		if (debug_global){ printf("PARSE: Adding parameter: %s\n", commands[1]); }
		params = (char*)malloc(strlen(commands[1] + 1));
		if (params == NULL){
			fprintf(stderr, "PARSE: Error during malloc!");
		}
		strcpy(params, commands[1]);
		
		while (commands[i]){
			if (debug_global){ printf("PARSE: Adding parameter: %s\n", commands[i]); }
			params = concat_string(params, " ", commands[i]);
			i++;
		}
	}
	if (debug_global){ printf("PARSE: Sending %s to parse_command for execution\n", commands[0]); }
	parse_command(commands[0], params, type);
}
