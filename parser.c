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
#include <tchar.h>
#include <Windows.h>
#include "parser.h"

/*Parses a single command which exists*/
void parse_command(wchar_t *command, wchar_t **params) {
	int i = 0;

	// Stuff for the createprocess
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Printing the parameters (debugging)
	while (params[i]){
		printf("With parameters: %s\n", params[i]);
		i++;
	}
	
	// This stuff is broken, I don't even know, some stupid shit about wide chars. Putting an L before a literal string does work but I can't put an L before my wchar_t (or can i?)
	printf("%s\n", command);

	// Create the process
	if (!CreateProcess(command, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Error! %d\n", GetLastError());
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

/* Splits a string of space seperated words into an array of words*/
char **split(wchar_t *str) {
	wchar_t *token;
	wchar_t **commands = 0;
	wchar_t *newline;

	int count = 0;

	token = strtok(str, " ");

	while (token) {
		// Remove newline character
		newline = strchr(token, '\n');
		if (newline) {
			*newline = 0;
		}

		commands = realloc(commands, sizeof(wchar_t*)* ++count);
		commands[count - 1] = token;
		token = strtok(0, " ");
	}

	//Add a null entry to the end of the array
	commands = realloc(commands, sizeof (wchar_t*)* (count + 1));
	commands[count] = 0;

	return commands;
}

/*Processes a line of commands*/
void parse(wchar_t *cmdline) {
	wchar_t **commands;
	wchar_t **params;
	int i = 0;
	int j = 0;
	int index = 0;

	commands = split(cmdline);
	params = malloc((sizeof(wchar_t) * 5) * 20);

	// While there are tokens left...
	while (commands[i]) {

		// If a token isn't a parameter
		if (commands[i][0] != '-'){
			//Check the following tokens for parameters
			j = i + 1;
			index = 0;
			while (commands[j] && commands[j][0] == '-') {
				params[index] = commands[j];
				index++;
				j++;
			}

			// Add null value
			params = realloc(params, sizeof (wchar_t*)* (index + 1));
			params[index] = 0;

			// Parse the command and params
			parse_command(commands[i], params);
		}

		// Move to next token
		i++;
	}
}

void print_info() {
}

void free_info() {
}

