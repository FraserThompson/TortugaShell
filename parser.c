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

/* Splits a string of space seperated words into an array of words*/
char **split (char str[]) {
	char *token = strtok(str, " ");
	char **commands = NULL;
	int count = 0, i;

	while (token) {
		commands = realloc(commands, sizeof(char*)* ++count);
		commands[count - 1] = token;
		token = strtok(0, " ");
	}

	printf("Adding null to end...\n");
	commands = realloc(commands, sizeof (char*)* (count + 1));
	commands[count] = 0;
	
	printf("%s\n", commands[0]);
	return commands;
}

/*Parses a command*/
void parse_command (char *command, parseInfo *p) {
}

/*Parses a line of commands*/
void *parse (char cmdline[]) {
	char **commands = split(cmdline);
	int i = 0;
	while (commands[i]) {
		printf("%s\n", commands[i++]);
		// Check to see if the command exists, if it does get the parseInfo struct and send it to parse_command
	}
}

void print_info (parseInfo *info) {
}

void free_info (parseInfo *info) {
}

int main (int argc, char *argv[]) {
	char test[] = "this is a test"; 
	parse(test);
	return 0;
}

