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

HANDLE hChildStdoutRd = NULL;
HANDLE hChildStdoutWr = NULL;
HANDLE hChildStdinRd = NULL;
HANDLE hChildStdinWr = NULL;
HANDLE hStdout = NULL;

/* -------WINDOWS------
* Converts a normal array of char into an array of wide char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
wchar_t *convert_to_wchar(char *input){
	size_t len = strlen(input) + 1;
	wchar_t *command_w = malloc(sizeof(wchar_t)* len);
	if (debug_global > 1){ printf("CONVERT_TO_WCHAR: Input - %ws\n", input); }

	if (command_w == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	swprintf(command_w, len, L"%hs", input);
	if (debug_global > 1){ printf("CONVERT_TO_WCHAR: Output - %s\n", command_w); }
	return command_w;
}

/* -------WINDOWS------
* Converts an array of widechar into an array of char because Windows
* Parameter: String to convert
* Return: Wchar version of input
*/
char *convert_to_char(wchar_t *input){
	size_t len = wcslen(input) + 1;
	if (debug_global > 1){ printf("CONVERT_TO_CHAR: Input - %ws\n", input); }
	char *command_c = malloc(sizeof(char)* len);

	if (command_c == NULL){
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	wcstombs(command_c, input, len);
	if (debug_global > 1){ printf("CONVERT_TO_CHAR: Output - %s\n", command_c); }
	return command_c;
}

/* -------WINDOWS------
* Returns the path to the system dir.
* Return: path to the system dir
*/
char *get_system_dir(void){
	size_t size = 100;
	wchar_t buffer[100];
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

/* -------WINDOWS------
* Writes to a child processes pipe
* Parameters: String to write
* Return: Error code, 0 if success.
*/
int write_to_pipe(char *content){
	DWORD dwWritten;
	char chBuf[256];
	strcpy(chBuf, content);

	if (!WriteFile(hChildStdinWr, chBuf, 256, &dwWritten, NULL)){
		fprintf(stderr, "WRITE_TO_PIPE: Error writing to pipe.\n");
		return EXIT_FAILURE;
	}
	if (debug_global){ printf("WRITE_TO_PIPE: Writing %s to pipe...\n", chBuf); }


	if (!CloseHandle(hChildStdinWr)){
		fprintf(stderr, "WRITE_TO_PIPE: Error closing pipe.\n");
		return EXIT_FAILURE;
	}

	if (debug_global){ printf("WRITE_TO_PIPE: Done\n"); }
}

/* -------WINDOWS------
* Reads from a child processes pipe
* Parameters: String to write
* Return: Error code, 0 if success.
*/
char* read_from_pipe(){
	DWORD dwRead, dwWritten;
	CHAR chBuf[256];

	if (!CloseHandle(hChildStdoutWr)){
		fprintf(stderr, "READ_FROM_PIPE: Error closing pipe.\n");
	}
	for (;;){
		if (!ReadFile(hChildStdoutRd, chBuf, 256, &dwRead, NULL) || dwRead == 0) break;
		if (!WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL)) break;
	}
	return chBuf;
}

/* -------WINDOWS------
* Creates a process in Windows.
* Parameters: Location of process to spawn, parameters
* Return: Error code, 0 if success.
*/
int create_process(char *command, char *params, char *redirectIn, char *redirectOut) {
	int error = 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES as;
	wchar_t *param_wchar = NULL;
	wchar_t *command_wchar = NULL;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	
	si.cb = sizeof(si);
	si.hStdError = hChildStdoutWr;
	si.hStdOutput = hChildStdoutWr;
	si.hStdInput = hChildStdinRd;
	si.dwFlags = STARTF_USESTDHANDLES;

	as.nLength = sizeof(as);
	as.lpSecurityDescriptor = NULL;
	as.bInheritHandle = TRUE;

	if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &as, 0)){
		fprintf(stderr, "CREATE_PROCESS: Failed to create stdout pipe.\n");
	}

	if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &as, 0)){
		fprintf(stderr, "CREATE_PROCESS: Failed to create stdin pipe.\n");
	}


	if (debug_global){ printf("CREATE_PROCESS: Creating process in Windows %s with parameter %s\n", command, params); }

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

