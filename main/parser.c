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
* Parameter: String to split, delimiter, memory address of integer to store index of last item.
* Return: Array of words
*/
char **split(char *str, char *delimiter, int *last_index) {
	char *token;
	char **commands = 0;
	char *newline;
	int count = 0;

	token = strtok(str, delimiter);

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
		token = strtok(0, delimiter);
		if (debug_global){ printf("SPLIT: Done with token: %s\n", commands[count - 1]); }
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (char*)* (count + 1));

	if (commands == NULL){
		fprintf(stderr, "SPLIT: Error during realloc!\n");
		exit(EXIT_FAILURE);
	}

	commands[count] = 0;
	if (debug_global){ printf("SPLIT: Returning %i tokens\n", count); }
	*last_index = count;
	return commands;
}


/* -----CROSS PLATFORM----
* Parses a single command
* Parameters: Command to parse, parameters string, type of command)
*/
void parse_command(char *command, char *params, int type) {
	int error = 0;
	int i = 0;
	char *command_dir; // Command with dir on front
        char *argv;
	char *command_ext; // Command with ext on end
	char *path_commands = get_commands_dir(); // PATH/commands/
	char *system_dir = get_system_dir(); // /bin in linux, C:/windows/system32 in windows
	char *dirs[NUM_DIRS] = { path_commands, system_dir, "./"};

	if (debug_global){ printf("PARSE_COMMAND: Input: %s %s %i\n", command, params, type); }

	// Processing a relative path
	if (type == 0) {

		// Check for the desired command in all dirs until found, also check with the extension
		while (i != NUM_DIRS){
			command_dir = concat_string(dirs[i], command, NULL); // Add directory to front
                        argv = command_dir; // Set the argument as the path to the command
			command_ext = get_command_ext(command_dir); // Add extension to end
			i++;
                        
                        // If there's a parameter add it on to the argv string
                        if (params){
                            argv = concat_string(argv, " ", params);
                        }
                        
			if (debug_global){ printf("PARSE_COMMAND: Trying to create %s as a process with params %s\n", command_dir, params); }

			error = create_process(command_dir, argv);
                        
			// If it's all good
			if (error == 0) {
				return;
			}

			// If it's not all good
			else {
				if (debug_global){ printf("PARSE_COMMAND: Unable to create process error %i\n", error); }
				// Try again with the extension
				if (debug_global){ printf("PARSE_COMMAND: Trying again with extension on the end\n"); }
				error = create_process(command_ext, params); 
				// If it's all good
				if (error == 0) {
					return;
				}
				// If it's not all good
				else {
					if (debug_global){ printf("PARSE_COMMAND: Unable to create process error %i\n", error); }
				}

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
	int last_index = 0;
	int i = 2;

	if (debug_global){ printf("PARSE: Input: %s\n", cmdline); }
	commands = split(cmdline, " ", &last_index);
	if (debug_global){ printf("PARSE: First command: %s\n", commands[0]); }

        /* Begin internal commands! */
	// cwd
	if (strcmp(commands[0], "cwd") == 0){
		if (debug_global){ printf("PARSE: Got cwd, printing cwd then returning.\n"); }
		printf("%s\n", getCWD(commands[1]));
		return;
	}
	// help
	else if (strcmp(commands[0], "help") == 0){
		if (debug_global){ printf("PARSE: Got help, printing help then returning.\n"); }
		print_help();
		return;
	}
        /* End internal commands! */
        
	type = command_type(commands[0]);

	// If there's more than just one thing
	if (commands[1]){
		// If the user tried to cd
		if (strcmp(commands[0], "cd") == 0){
			if (debug_global){ printf("PARSE: Got CD, changing directory then returning.\n"); }
			cd(commands[1]);
			return;
		}
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
	if (debug_global){ printf("PARSE: Sending %s to parse_command forextcution\n", commands[0]); }
	parse_command(commands[0], params, type);
}
