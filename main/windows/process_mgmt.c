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
#define BUFSIZE 4096

HANDLE child_out_read = NULL;
HANDLE child_out_write = NULL;
HANDLE child_in_read = NULL;
HANDLE child_in_write = NULL;

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

/* -------WINDOWS------
* Writes to a child processes pipe
* Parameters: String to put into the pipe
* Return: Error code, 0 if success.
*/
int write_to_pipe(char *content){
	DWORD dwWritten;
	char chBuf[BUFSIZE];
	strcpy(chBuf, content);

	if (!WriteFile(child_in_write, chBuf, BUFSIZE, &dwWritten, NULL)){
		fprintf(stderr, "WRITE_TO_PIPE: Error writing to pipe.\n");
		return EXIT_FAILURE;
	}
	if (debug_global){ printf("WRITE_TO_PIPE: Writing %s to pipe...\n", chBuf); }


	if (!CloseHandle(child_in_write)){
		fprintf(stderr, "WRITE_TO_PIPE: Error closing pipe.\n");
		return EXIT_FAILURE;
	}

	if (debug_global){ printf("WRITE_TO_PIPE: Done\n"); }
}

/* -------WINDOWS------
* Reads from a child processes pipe
* Parameters: Location to read to
* Return: Error code, 0 if success.
*/
void read_from_pipe(out_file){
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL success = FALSE;
	HANDLE parent_out = NULL;

	if (!out_file){
		parent_out = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	else {
		// Open handle to output file
		parent_out = CreateFile(out_file, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	}

	// Close write end of pipe before reading read end
	if (!CloseHandle(child_out_write)){
		fprintf(stderr, "READ_FROM_PIPE: Error closing pipe.\n");
		return;
	}

	// Start reading
	for (;;){
		success = ReadFile(child_out_read, chBuf, BUFSIZE, &dwRead, NULL);
		if (!success || dwRead == 0) {
			if (GetLastError() == 109){
				if (debug_global){ printf("READ_FROM_PIPE: Finished.\n"); }
				return;
			}
			fprintf(stderr, "READ_FROM_PIPE: Error %u when reading pipe.\n", GetLastError());
			break;
		}
		else {
			chBuf[dwRead] = NULL;
			if (debug_global){ printf("READ_FROM_PIPE: Successfully read '%s' from childs standard output pipe.\n", chBuf); }
		}
		success = WriteFile(parent_out, chBuf, dwRead, &dwWritten, NULL);
		if (!success) {
			fprintf(stderr, "READ_FROM_PIPE: Error %u when writing to output pipe\n", GetLastError());
			break;
		}
		else {
			if (debug_global){ printf("READ_FROM_PIPE: Succesfully wrote '%s' to output pipe\n", chBuf); }
		}
	}
	if (!CloseHandle(child_in_write))
		fprintf(stderr, "READ_FROM_PIPE: Error %u when closing handle.", GetLastError());
	else {
		if (debug_global){ printf("READ_FROM_PIPE: Pipe handle closed.\n"); }
	}
}

/* -------WINDOWS------
* Goes through the process of opening handles, creating a process and handling the pipes.
* Parameters: command_line struct
* Return: Error code, 0 if success.
*/
int create_process(command_line line) {
	int error = 0;
	SECURITY_ATTRIBUTES sa;
	wchar_t *param_wchar = NULL;
	wchar_t *command_wchar = NULL;
	wchar_t *redirectIn_wchar = NULL;
	wchar_t *redirectOut_wchar = NULL;

	// Security Attributes
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (debug_global){ printf("\n*CREATE_PROCESS: Parent process ID %u\n", GetCurrentProcessId()); }

	// Create pipe for child process stdout
	if (!CreatePipe(&child_out_read, &child_out_write, &sa, 0)){
		fprintf(stderr, "CREATE_PROCESS: Failed to create stdout pipe.\n");
	}
	else {
		if (debug_global){ printf("CREATE_PROCESS: Stdout pipe created.\n"); }
	}
	SetHandleInformation(child_in_write, HANDLE_FLAG_INHERIT, 0);

	// Create pipe for child process stdin
	if (!CreatePipe(&child_in_read, &child_in_write, &sa, 0)){
		fprintf(stderr, "CREATE_PROCESS: Failed to create stdin pipe.\n");
	}
	else {
		if (debug_global){ printf("CREATE_PROCESS: Stdin pipe created.\n"); }
	}
	SetHandleInformation(child_out_read, HANDLE_FLAG_INHERIT, 0);

	// Conversion stuff
	if (line.params){
		param_wchar = convert_to_wchar(line.params);
	}
	if (line.command){
		command_wchar = convert_to_wchar(line.command);
	}

	/*if (line.redirectIn){
		redirectIn_wchar = convert_to_wchar(line.redirectIn);
	}

	if (line.redirectOut){
		redirectOut_wchar = convert_to_wchar(line.redirectOut);
	}*/

	//WaitForSingleObject(pi.hProcess, INFINITE);
	// Spawn process
	error = create_child(command_wchar, param_wchar);
	// Read from pipe
	read_from_pipe(line.redirectOut);

	return error;
}

/* -------WINDOWS------
* Actually spawns a process. Helper for createprocess.
* Parameters: command, args
* Return: Error code, 0 if success.
*/
static int create_child(wchar_t *command, wchar_t *param){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL success = FALSE;
	int error = 0;

	ZeroMemory(&pi, sizeof(pi));

	// Startup info
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = child_out_write;
	si.hStdOutput = child_out_write;
	si.hStdInput = child_in_read;
	si.dwFlags |= STARTF_USESTDHANDLES;

	// Spawn process
	success = CreateProcess(command, param, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (debug_global){ printf("CREATE_CHILD: Creating process in Windows %ws with parameter %ws\n", command, param); }
	if (!success){
		error = GetLastError();
	}
	else {
		if (debug_global) { printf("CREATE_CHILD: Child process ID: %u\n", GetCurrentProcessId()); }
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}