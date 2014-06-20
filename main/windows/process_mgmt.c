/*
* process_mgmt.c
*
*  Created on: 11/05/2014
*      Author: Fraser
*  
*  Contains methods which help with creating processes on Windows.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <wchar.h>
#include "../process_mgmt.h"
#include "../parser.h"

/* -------WINDOWS------
* Converts a normal array of char into an array of wide char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
wchar_t *convert_to_wchar(char *input){
	size_t len = strlen(input) + 1;
	wchar_t *command_w = malloc(sizeof(wchar_t)* len);

	if (command_w == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	swprintf(command_w, len, L"%hs", input);
	return command_w;
}

/* -------WINDOWS------
* Converts an array of widechar into an array of char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
char *convert_to_char(wchar_t *input){
	size_t len = wcslen(input) + 1;
	if (debug_global){ printf("CONVERT_TO_CHAR: Input - %ws\n", input); }
	char *command_c = malloc(sizeof(char)* len);

	if (command_c == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	wcstombs(command_c, input, len);
	if (debug_global){ printf("CONVERT_TO_CHAR: Output - %s\n", command_c); }
	return command_c;
}

/* -------WINDOWS------
* Returns the path to the system dir.
* Return: path to the system dir
*/
char *get_system_dir(void){
	size_t size = 100;
	wchar_t buffer[100];
	if (debug_global){ printf("GET_SYSTEM_DIR: Getting system dir...\n"); }
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

/* -------WINDOWS------
* Creates a process in Windows.
* Parameters: Location of process to spawn, parameters
* Return: Error code, 0 if success.
*/
int create_process(char *command, char *params) {
	int error = 0;
	wchar_t *param_wchar = NULL;
	wchar_t *command_wchar = NULL;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (debug_global){ printf("CREATE_PROCESS_WIN: Creating process in Windows %s with parameter %s\n", command, params); }

	if (params){
		param_wchar = convert_to_wchar(params);
	}

	if (command){
		command_wchar = convert_to_wchar(command);
	}

	if (!CreateProcess(command_wchar, param_wchar, NULL, NULL, 0, 0, NULL, NULL, &si, &pi)){
		error = GetLastError();
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return error;
}
