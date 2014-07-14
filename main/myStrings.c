/*
* myStrings.c
*
*  Created on: 14/07/2014
*      Author: Fraser
*
*  Contains methods which help with string manipulation.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <wchar.h>
#include "myStrings.h"
#include "parser.h" //for debug_global

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
		if (debug_global > 1) { printf("CONCAT_STRING: Adding third: %s\n", third); }
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

/* -------WINDOWS------
* Converts a normal array of char into an array of wide char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
wchar_t *convert_to_wchar(char *input){
	if (debug_global > 1) { printf("CONVERT_TO_WCHAR: Input - %s\n", input); }

	size_t len = strlen(input) + 1;
	wchar_t *command_w = malloc(sizeof(wchar_t)* len);

	if (command_w == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	swprintf(command_w, len, L"%hs", input);
	if (debug_global > 1) printf("CONVERT_TO_WCHAR: Output - %ws\n", command_w);
	return command_w;
}

/* -------WINDOWS------
* Converts an array of widechar into an array of char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
char *convert_to_char(wchar_t *input){
	if (debug_global > 1){ printf("CONVERT_TO_CHAR: Input - %ws\n", input); }

	size_t len = wcslen(input) + 1;
	char *command_c = malloc(sizeof(char)* len);

	if (command_c == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	wcstombs(command_c, input, len);
	if (debug_global > 1){ printf("CONVERT_TO_CHAR: Output - %s\n", command_c); }
	return command_c;
}
