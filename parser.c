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
#include <Windows.h>
#include "parser.h"

/*Parses a single command which exists*/
void parse_command(char *command, char **params) {
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
	
	// Convert the char into a wide char because Windows
	wchar_t command_w[20];
	swprintf(command_w, 20, L"%hs", command);
	printf("%ws\n", command_w);

	// Create the process
	if (!CreateProcess(command_w, params[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) { //Command is the wrong sort of string so it doesn't work?
		printf("Error! %d\n", GetLastError());
		return;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

/* Splits a string of space seperated words into an array of words*/
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

/*Processes a line of commands*/
void parse(char *cmdline) {
	char **commands;
	char **params;
	int i = 0;
	int j = 0;
	int index = 0;

	commands = split(cmdline);
	params = malloc((sizeof(char) * 5) * 20);

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
			params = realloc(params, sizeof (char*)* (index + 1));
			params[index] = 0;

			// Parse the command and params
			parse_command(commands[i], params);
		}

		// Move to next token
		i++;
	}
}
