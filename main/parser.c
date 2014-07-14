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

	if (debug_global > 1){ printf("CONCAT_STRING: Input: %s %s\n", first, second); }
	if (third){
		if (debug_global > 1){ printf("CONCAT_STRING: Input: %s\n", third); }
		third_len = strlen(third) + 1;
	}

	char *result = (char*)malloc(first_len + second_len + third_len);

	if (result == NULL){
		fprintf(stderr, "CONCAT_STRING: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if (debug_global > 1){ printf("CONCAT_STRING: Adding first: %s\n", first); }
	strncpy(result, first, first_len);
	if (debug_global > 1){ printf("CONCAT_STRING: Adding second: %s\n", second); }
	strncat(result, second, second_len);

	if (third){
		if (debug_global > 1) { printf("CONCAT_STRING: Adding third: %s\n", third);  }
		strncat(result, third, third_len);
	}

	if (debug_global > 1) { printf("CONCAT_STRING: Returning: %s\n", result); }
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

	if (debug_global > 1){ printf("SPLIT: Input: %s\n", str); }

	while (token) {
		if (debug_global > 1){ printf("SPLIT: Working on token: %s\n", token); }

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
		if (debug_global > 1){ printf("SPLIT: Done with token: %s\n", commands[count - 1]); }
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (char*)* (count + 1));

	if (commands == NULL){
		fprintf(stderr, "SPLIT: Error during realloc!\n");
		exit(EXIT_FAILURE);
	}

	commands[count] = 0;
	if (debug_global > 1){ printf("SPLIT: Returning %i tokens\n", count); }
	*last_index = count;
	return commands;
}


/* -----CROSS PLATFORM----
* Parses a single command
* Parameters: Command to parse, parameters string, type of command)
*/
void parse_command(command_line line) {
	int error = 0;
	int i = 0;
	char *param_orig = line.params; //untouched params
	char *command_orig = line.command; //untouched command
	char *command_dir; // Command with dir on front
    char *argv; // String list of args
	char *command_ext; // Command with ext on end
	char *path_commands = get_commands_dir(); // PATH/commands/
	char *system_dir = get_system_dir(); // /bin in linux, C:/windows/system32 in windows
	char *dirs[NUM_DIRS] = { path_commands, system_dir, "./"};

	if (debug_global) printf("PARSE_COMMAND: Input: %s %s\n", line.command, line.params); 

	/* Processing a relative path */
	if (line.type == 0) {
		// Check for the desired command in all dirs until found, also check with the extension
		while (i != NUM_DIRS){
			command_dir = concat_string(dirs[i], command_orig, NULL); 
			line.params = command_dir; // First argument is always the path to the file
			line.command = command_dir;
			i++;
			// If there's a parameter add it on to the argv string
			if (param_orig){
				if (debug_global) printf("PARSE_COMMAND: There are parameters. Adding them to the argv.\n"); 
				line.params = concat_string(line.params, " ", param_orig);
			}

			error = create_process(line);

			// No errors
			if (error == 0) {
				return;
			}
			// Errors
			else {
				if (error == 50){
					fprintf(stderr, "PARSE_COMMAND: Redirect location is not accessible or does not exist.\n");
					return;
				}
				if (debug_global) printf("PARSE_COMMAND: Unable to create process error %i\n", error); 
				if (debug_global) printf("PARSE_COMMAND: Trying again with extension on the end\n");
				command_ext = get_command_ext(command_dir); 
				line.params = command_ext;
				line.command = command_ext;

				// If there's a parameter add it on to the argv string
				if (param_orig){
					if (debug_global) printf("PARSE_COMMAND: There are parameters. Adding them to the argv.\n");
					line.params = concat_string(line.params, " ", param_orig);
				}
				error = create_process(line);

				// No errors
				if (error == 0) {
					return;
				}
				// Errors
				else {
					if (debug_global){ printf("PARSE_COMMAND: Unable to create process error %i\n", error); }
				}

			}
		}

	}

	/* Processing an absolute path */
	if (line.type == 1){

		if (param_orig){
			argv = concat_string(line.command, " ", line.params);
		}
		else {
			argv = line.command;
		}

		line.params = argv;

		error = create_process(line);

		// No errors
		if (error == 0) {
			return;
		}
		else {
			if (error == 50){
				fprintf(stderr, "PARSE_COMMAND: Redirect location is not accessible or does not exist.\n");
				return;
			}
		}
	}

	printf("'%s' does not exist.\n", line.command);
	return;
}

/* -----CROSS PLATFORM----
* Processes a commandline
* Parameter: Line to process
*/
void parse(char *cmdline) {
	char **full_line;
	int last_index = 0;
	int i = 0;
	command_line line = { NULL, "", NULL, NULL, NULL, NULL };

	/*Split the raw line into tokens for processing*/
	if (debug_global){ printf("PARSE: Input: %s\n", cmdline); }
	full_line = split(cmdline, " ", &last_index);
	if (debug_global > 1){ printf("PARSE: First item: %s\n", full_line[0]); }

	/*First token will always be a command so add it to the struct and check type*/
	line.command = malloc(strlen(full_line[i] + 1) * sizeof(char));
	strcpy(line.command, full_line[i]);
	line.type = get_command_type(full_line[i]);

    /* Checking for internal commands */
	if (strcmp(line.command, "cwd") == 0){
		if (debug_global){ printf("PARSE: Got cwd, printing cwd.\n"); }
		printf("%s\n", getCWD());
		if (debug_global){ printf("PARSE: Done with cwd, continuing.\n"); }
		return;
	}
	else if (strcmp(line.command, "help") == 0){
		if (debug_global){ printf("PARSE: Got help, printing help.\n"); }
		print_help();
		if (debug_global){ printf("PARSE: Done with help, continuing.\n"); }
		return;
	}
 
	/* If there's more than one token */
	if (last_index > 1){
		/* Internal commands with params*/
		if (strcmp(line.command, "cd") == 0){
			if (debug_global){ printf("PARSE: Got cd, changing directory.\n"); }
			cd(full_line[1]);
			if (debug_global){ printf("PARSE: Done with cd, returning.\n"); }
			return;
		}

		i++; // increment i to skip the first token since we've already delt with it

		/* Process line and populate struct */
		while (full_line[i]){
			if (debug_global > 1) printf("PARSE: Working on item: %s\n", full_line[i]); 

			if (strcmp(full_line[i], ">") == 0){
				if (debug_global) printf("PARSE: Adding redirectOut location: %s\n", full_line[i + 1]); 
				line.redirectOut = malloc(strlen(full_line[++i]) * sizeof(char));
				strcpy(line.redirectOut, full_line[i]);
			}
			else if (strcmp(full_line[i], "<") == 0){
				if (debug_global) printf("PARSE: Adding redirectIn location: %s\n", full_line[i + 1]); 
				line.redirectIn = malloc(strlen(full_line[++i]) * sizeof(char));
				strcpy(line.redirectIn, full_line[i]);
			}
			else {
				if (debug_global) printf("PARSE: Adding parameter %s\n", full_line[i]); 
				line.params = concat_string(line.params, " ", full_line[i]);
			}
			i++;
		}
	}
	line.params = strcmp(line.params, "") != 0 ? line.params : NULL; //Set params to NULL if empty
	if (debug_global) printf("PARSE: Sending %s to parse_command for execution\n", line.command); 
	if (debug_global) display_info(line); 
	parse_command(line);
}

void display_info(command_line line){
	int i = 0;
	if (debug_global) printf("DISPLAY_INFO: Displaying info contained in line struct\n"); 
	printf("%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%-5s\n", "Command", "Argv", "redirectOut", "redirectIn", "pipe", "type");
	printf("%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%i\n", line.command, line.params, line.redirectOut, line.redirectIn, line.pipe, line.type);
}