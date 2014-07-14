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
* Parameters: Location of file to put into the pipe
* Return: Error code - 0 if success.
*/
int write_to_pipe(HANDLE inputFile){
	DWORD dwWritten, dwRead;
	int error;
	char chBuf[BUFSIZE];

	for (;;)
	{
		if (!ReadFile(inputFile, chBuf, BUFSIZE, &dwRead, NULL) || dwRead == 0) {
			error = GetLastError();
			if (error == 0){
				break;
			}
			fprintf(stderr, "WRITE_TO_PIPE: Error %u when reading file.\n", error);
			break;
		}

		chBuf[dwRead] = NULL; //add terminating character
		if (debug_global) printf("WRITE_TO_PIPE: Writing %s to pipe.\n", chBuf);

		if (!WriteFile(child_in_write, chBuf, dwRead, &dwWritten, NULL)) {
			error = GetLastError();
			fprintf(stderr, "WRITE_TO_PIPE: Error %u when writing file to pipe.\n", error);
			break;
		}
	}

	if (!CloseHandle(child_in_write)){
		fprintf(stderr, "WRITE_TO_PIPE: Error closing pipe.\n");
		return EXIT_FAILURE;
	}

	return error;
}

/* -------WINDOWS------
* Reads from a child processes pipe and write to a specified file
* Parameters: Location to write to
* Return: Error code - 0 if success
*/
int read_from_pipe(out_file){
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL success = FALSE;
	HANDLE parent_out = NULL;
	int error = 0;

	// Open handle to output file
	parent_out = CreateFile(out_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Close write end of pipe before reading read end
	if (!CloseHandle(child_out_write)){
		fprintf(stderr, "READ_FROM_PIPE: Error closing pipe.\n");
		return 1;
	}

	// Start reading each thing in the thing and writing to the place
	for (;;){
		success = ReadFile(child_out_read, chBuf, BUFSIZE, &dwRead, NULL);
		if (!success || dwRead == 0) {
			error = GetLastError();
			if (error == 109){
				if (debug_global){ printf("READ_FROM_PIPE: Finished.\n"); }
				break;
			}
			fprintf(stderr, "READ_FROM_PIPE: Error %u when reading pipe.\n", error);
			break;
		}
		else {
			chBuf[dwRead] = NULL;
			if (debug_global){ printf("READ_FROM_PIPE: Successfully read '%s' from childs standard output pipe.\n", chBuf); }
		}
		success = WriteFile(parent_out, chBuf, dwRead, &dwWritten, NULL);
		if (!success) {
			error = GetLastError();
			fprintf(stderr, "READ_FROM_PIPE: Error %u when writing to output pipe\n", error);
			break;
		}
		else {
			if (debug_global){ printf("READ_FROM_PIPE: Succesfully wrote '%s' to output pipe\n", chBuf); }
		}
	}

	if (!CloseHandle(parent_out)){
		fprintf(stderr, "READ_FROM_PIPE: Error %u when closing output handle.", GetLastError());
	}
	else {
		if (debug_global){ printf("READ_FROM_PIPE: Output pipe handle closed.\n"); }
	}

	return error;
}

/* -------WINDOWS------
* Goes through the process of opening handles, creating a process and handling the pipes.
* Parameters: command_line struct
* Return: Error code, 0 if success, 50 if encountered a redirection error
*/
int create_process(command_line line) {
	int pError = 0;
	int rError = 0;
	SECURITY_ATTRIBUTES sa;
	HANDLE inputFile = NULL;
	wchar_t *param_wchar = NULL;
	wchar_t *command_wchar = NULL;
	wchar_t *redirectIn_wchar = NULL;
	wchar_t *redirectOut_wchar = NULL;

	// Security Attributes
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (debug_global){ printf("\nCREATE_PROCESS: Parent process ID %u\n", GetCurrentProcessId()); }
	if (debug_global) { printf("CREATE_PROCESS: Parent thread ID: %u\n", GetCurrentThreadId()); }
	if (debug_global > 1){ display_info(line); }

	// Create pipe for child process stdout
	if (line.redirectOut){
		if (!CreatePipe(&child_out_read, &child_out_write, &sa, 0)){
			fprintf(stderr, "CREATE_PROCESS: Failed to create stdout pipe.\n");
			return 3;
		}
		else {
			if (debug_global){ printf("CREATE_PROCESS: Stdout pipe created.\n"); }
		}
		SetHandleInformation(child_out_read, HANDLE_FLAG_INHERIT, 0);
		redirectOut_wchar = convert_to_wchar(line.redirectOut);
	}

	// Create pipe for child process stdin and open handle to file
	if (line.redirectIn){
		if (!CreatePipe(&child_in_read, &child_in_write, &sa, 0)){
			fprintf(stderr, "CREATE_PROCESS: Failed to create stdin pipe.\n");
			return 50;
		}
		else {
			if (debug_global){ printf("CREATE_PROCESS: Stdin pipe created.\n"); }
		}
		SetHandleInformation(child_in_write, HANDLE_FLAG_INHERIT, 0);
		redirectIn_wchar = convert_to_wchar(line.redirectIn);

		inputFile = CreateFile(redirectIn_wchar, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if (inputFile == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "CREATE_PROCESS: Failed to create handle to input file '%ws'. Does it exist?\n", redirectIn_wchar);
			return 50;
		}
		else {
			if (debug_global) printf("CREATE_PROCESS: Handle to file '%ws' created.\n", redirectIn_wchar);
		}
	}

	// Conversion stuff
	if (line.params){
		if (debug_global > 1) printf("CREATE_PROCESS: Sending '%s' to be converted to wchar.\n", line.params);
		param_wchar = convert_to_wchar(line.params);
	}
	if (line.command){
		if (debug_global > 1) printf("CREATE_PROCESS: Sending '%s' to be converted to wchar.\n", line.command);
		command_wchar = convert_to_wchar(line.command);
	}

	// Spawn process
	pError = create_child(command_wchar, param_wchar, line.redirectOut, line.redirectIn);

	// Don't continue if the process wasn't created succesfully 
	if (pError != 0){
		clean_up();
		return pError;
	}

	// Write to the childs input buffer, it'll pick it up
	if (line.redirectIn){
		rError = write_to_pipe(inputFile);
		// Don't continue if it couldn't redirect properly
		if (rError != 0 && rError != 109){
			fprintf(stderr, "CREATE_PROCESS: Input redirection error %i. Halting.\n", rError);
			pError = 50;
		}
	}

	// Read from the childs output buffer
	if (line.redirectOut){
		rError = read_from_pipe(redirectOut_wchar);
		// Don't continue if it couldn't redirect properly
		if (rError != 0 && rError != 109){
			fprintf(stderr, "CREATE_PROCESS: Output redirection error %i. Halting.\n", rError);
			pError = 50;
		}
	}

	clean_up();
	return pError;
}

static int clean_up(void){

	if (!CloseHandle(child_in_write)) {
		if (debug_global > 1) printf("CLEAN_UP: Error %u when closing child input write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) printf("CLEAN_UP: Child input write pipe handle closed.\n"); 
	}

	if (!CloseHandle(child_out_write)) {
		if (debug_global > 1) printf("CLEAN_UPE: Error %u when closing child output write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) printf("CLEAN_UP: Child output write pipe handle closed.\n");
	}


	if (!CloseHandle(child_in_read)) {
		if (debug_global > 1) printf("CLEAN_UP: Error %u when closing child input read handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) printf("CLEAN_UP: Child input read pipe handle closed.\n");
	}

	if (!CloseHandle(child_out_read)) {
		if (debug_global > 1) printf("CLEAN_UP: Error %u when closing child output read handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) printf("CLEAN_UP: Child output read pipe handle closed.\n");
	}
}

/* -------WINDOWS------
* Actually spawns a process. Helper for createprocess.
* Parameters: command, args
* Return: Error code, 0 if success.
*/
static int create_child(wchar_t *command, wchar_t *param, char *redirectOut, char *redirectIn){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL success = FALSE;
	int error = 0;

	ZeroMemory(&pi, sizeof(pi));

	// Startup info
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (redirectIn){
		si.hStdInput = child_in_read;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	if (redirectOut){
		si.hStdOutput = child_out_write;
		si.hStdError = child_out_write;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	// Spawn process
	success = CreateProcess(command, param, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (debug_global){ printf("CREATE_CHILD: Creating process in Windows %ws with parameter %ws\n", command, param); }
	if (!success){
		error = GetLastError();
	}
	else {
		WaitForSingleObject(pi.hProcess, INFINITE);
		if (debug_global) { printf("CREATE_CHILD: Child process ID: %u\n", GetCurrentProcessId()); }
		if (debug_global) { printf("CREATE_CHILD: Child thread ID: %u\n", GetCurrentThreadId()); }
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return error;
}