/*
* process_mgmt.c
*
*  Created on: 11/05/2014
*      Author: Fraser
*  
*  Contains methods which help with creating processes on Windows/Linux.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

/* -------WINDOWS------
* Converts a normal array of char into a wide char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
static wchar_t *convert_to_wchar(char *input){
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
* Creates a process in Windows.
* Parameters: Location of process to spawn, parameters
* Return: Error code, 0 if success.
*/
int create_process_win(char *command, char *params) {
	int error = 0;
	wchar_t *param_wchar = NULL;
	wchar_t *command_wchar = NULL;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (params){
		param_wchar = convert_to_wchar(params);
	}

	if (command){
		command_wchar = convert_to_wchar(command);
	}

	if (!CreateProcess(command_wchar, param_wchar, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
		error = GetLastError();
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return error;
}

/* -------UNIX------
* Creates a process in Unix/Linux.
* Parameters: Location of process to spawn
* Return: Error code, 0 if success.
*/
int create_process_unix(char *command, char *params) {
}
