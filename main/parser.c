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
#include <Windows.h>
#include <string.h>
#include "cwd.h"
#include "cd.h"
#include "help.h"
#include "parser.h"
#include "myStrings.h"
#include "process_mgmt.h"
#define BUFSIZE 4096

/* -----CROSS PLATFORM----
* Parses a single command processed by parse()
* Parameters: Command to parse, parameters string, type of command)
*/
static void parse_command(command_line line) {
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

/* -------CROSS-PLATFORM------
* Prints the info contained in the command_line struct
* Param: command_line struct
*/
void display_info(command_line line){
	int i = 0;
	if (debug_global) printf("DISPLAY_INFO: Displaying info contained in line struct\n"); 
	printf("%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%-5s\n", "Command", "Argv", "redirectOut", "redirectIn", "pipe", "type");
	printf("%-10s\t%-5s\t%-10s\t%-10s\t%-10s\t%i\n", line.command, line.params, line.redirectOut, line.redirectIn, line.pipe, line.type);
}

/* -------WINDOWS------
* Returns the path to the system dir.
* Return: path to the system dir
*/
char *get_system_dir(void){
	size_t size = 100;
	wchar_t buffer[BUFSIZE];
	if (debug_global > 1){ printf("GET_SYSTEM_DIR: Getting system dir...\n"); }
	if (!GetSystemDirectory(buffer, size)){
		printf("GET_SYSTEM_DIR: Error getting system dir!\n");
		exit(EXIT_FAILURE);
	}
	return concat_string(convert_to_char(buffer), "\\", NULL);
}

/* -------WINDOWS------
* Returns the PATH the application was run from with \\commands\\ on the end.
* Return: path
*/
char *get_commands_dir(void){
	return concat_string(PATH, "\\commands\\", NULL);
}

/* -------WINDOWS------
* Returns the command with the extension added (.exe in Windows).
* Parameter: Command to attach it to.
* Return: Command with extension added
*/
char *get_command_ext(char *command){
	return concat_string(command, ".exe", NULL);
}

/* -----WINDOWS----
* Check to see what the command type is: Whether it's just a single command or a physical path.
* This is Windows so it does this by checking if the second character is a semicolon. Will have to be different for Linux.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
int get_command_type(char *command){
	if (debug_global){ printf("GET_COMMAND_TYPE: Input: %s\n", command); }
	if (command[1] == ':'){
		if (debug_global){ printf("GET_COMMAND_TYPE: It's a path.\n"); }
		return 1;
	}
	if (debug_global){ printf("GET_COMMAND_TYPE: It's a command.\n"); }
	return 0;
}