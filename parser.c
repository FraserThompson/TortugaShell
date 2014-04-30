/*
 * parser.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */
#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdlib.h>
#include "parser.h"

/*Parses a single command which exists*/
void parse_command(char *command) {
	//exec and all that
}

/* Splits a string of space seperated words into an array of words*/
char **split(char str[]) {
	char *token = strtok(str, " ");
	char **commands = 0;
	int count = 0, i;

	while (token) {
		commands = realloc(commands, sizeof(char*)* ++count);
		commands[count - 1] = token;
		token = strtok(0, " ");
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (char*)* (count + 1));
	commands[count] = 0;

	return commands;
}

/*Processes a line of commands*/
void *parse(char cmdline[]) {
	char **commands = split(cmdline);
	int i = 0;
	int j;

	while (commands[i]) {
		//check to see if it's a thing first
		//check if it begins with a dash as well
		parse_command(commands[i++]);
	}
}

void print_info() {
}

void free_info() {
}

