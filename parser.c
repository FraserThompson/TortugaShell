/*
 * parser.c
 *
 *  Created on: 14/04/2014
 *      Author: Fraser
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>


char split (char *string) {
	char delimiter[] = " ";
	char *result;
	char strArray[10];
	int count = 0;

	result = srtok_r(string, delimiter);

	while (result) {
		strcpy(strArray[count++], result);
		result = strtok_r(0, delimiter);
	}

	return strArray;
}

void parse_command (char *command, parseInfo *p) {
}

void *parse (char *cmdline) {
	char *commands = split(*cmdline);
	printf("%s\n", commands[0]);
}

void print_info (parseInfo *info) {
}

void free_info (parseInfo *info) {
}

int win_main (int argc, char *argv[]) {
	parse("This is a test");
	return 0;
}

