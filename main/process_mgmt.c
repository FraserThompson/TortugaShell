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
#include "process_mgmt.h"
#include "shell.h"
#include "myStrings.h"
#include "parser.h"
#define BUFSIZE 4096
#define NUM_DIRS 3

HANDLE child_out_read;
HANDLE child_out_write;
HANDLE child_in_read;
HANDLE child_in_write;
HANDLE inputFile;
SECURITY_ATTRIBUTES sa;

/* -------WINDOWS------
* Writes to a child processes pipe
* Parameters: Location of file to put into the pipe
* Return: Error code - 0 if success.
*/
static int write_to_pipe(HANDLE inputFile){
	DWORD dwWritten, dwRead;
	int error;
	CHAR chBuf[BUFSIZE];

	for (;;)
	{
		if (!ReadFile(inputFile, chBuf, BUFSIZE, &dwRead, NULL) || dwRead == 0) {
			error = GetLastError();
			if (error == 0){
				break;
			}
			fwprintf(stderr, L"WRITE_TO_PIPE: Error %u when reading file.\n", error);
			break;
		}

		chBuf[dwRead] = '\0'; //add terminating character
		if (debug_global) printf("WRITE_TO_PIPE: Writing %s to pipe.\n", chBuf);

		if (!WriteFile(child_in_write, convert_to_char(chBuf), dwRead, &dwWritten, NULL)) {
			error = GetLastError();
			fwprintf(stderr, L"WRITE_TO_PIPE: Error %u when writing file to pipe.\n", error);
			break;
		}
	}

	if (!CloseHandle(child_in_write)){
		fwprintf(stderr, L"WRITE_TO_PIPE: Error closing pipe.\n");
		return EXIT_FAILURE;
	}


	if (!CloseHandle(child_out_write)) {
		if (debug_global > 1) wprintf(L"CLEAN_UPE: Error %u when closing child output write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child output write pipe handle closed.\n");
	}

	if (!CloseHandle(inputFile)){
		fwprintf(stderr, L"WRITE_TO_PIPE: Error closing input file handle.\n");
		return EXIT_FAILURE;
	}
	return error;
}

/* -------WINDOWS------
* Reads from a child processes pipe and write to a specified file or memory location
* Parameters: Location to write to, pointer to allocated wchar_t memory to hold stdout
* Return: Error code - 0 if success
*/
static int read_from_pipe(wchar_t *out_file, wchar_t **variable){
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	wchar_t *chBuf_w = emalloc(sizeof(wchar_t)* BUFSIZE);
	BOOL success = FALSE;
	HANDLE parent_out = NULL;
	int error = 0;
	int isVar = wcscmp(out_file, L":var:");

	// Open handle to output file unless we want to return a variable
	if (isVar != 0){
		parent_out = CreateFileW(out_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	// Close write end of pipe before reading read end
	if (!CloseHandle(child_out_write)){
		fwprintf(stderr, L"READ_FROM_PIPE: Error closing pipe.\n");
		return 1;
	}

	// Start reading each thing in the thing and writing to the place
	for (;;){
		success = ReadFile(child_out_read, chBuf, BUFSIZE, &dwRead, NULL);
		if (!success || dwRead == 0) {
			error = GetLastError();
			if (error == 109){
				if (debug_global){ wprintf(L"READ_FROM_PIPE: Finished.\n"); }
				break;
			}
			fwprintf(stderr, L"READ_FROM_PIPE: Error %u when reading pipe.\n", error);
			break;
		}
		else {
			chBuf[dwRead] = '\0';
			if (debug_global){ printf("READ_FROM_PIPE: Successfully read '%s' from childs standard output pipe.\n", chBuf); }

		}

		if (isVar != 0) {
			success = WriteFile(parent_out, chBuf, dwRead, &dwWritten, NULL);
			if (!success) {
				error = GetLastError();
				fwprintf(stderr, L"READ_FROM_PIPE: Error %u when writing to output pipe\n", error);
				break;
			}
			else {
				if (debug_global){ printf("READ_FROM_PIPE: Succesfully wrote '%s' to output pipe\n", chBuf); }
			}
		}
		else {
			chBuf_w = convert_to_wchar(chBuf);
			wcscpy(variable, chBuf_w);
		}
	}

	if (isVar != 0){
		if (!CloseHandle(parent_out)){
			fwprintf(stderr, L"READ_FROM_PIPE: Error %u when closing output handle.", GetLastError());
		}
		else {
			if (debug_global){ wprintf(L"READ_FROM_PIPE: Output pipe handle closed.\n"); }
		}

	}

	return error;
}

/* -------WINDOWS------
* Opens a pipe between a child processes stdout and a parent process.
* Return: Error code, 0 if success
*/
static int open_output_pipe(){
	if (debug_global){ wprintf(L"OPEN_OUTPUT_PIPE: Opening output pipe...\n"); }
	if (!CreatePipe(&child_out_read, &child_out_write, &sa, 0)){
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create stdout pipe.\n");
		return 3;
	}
	else {
		if (debug_global){ wprintf(L"CREATE_PROCESS: Stdout pipe created.\n"); }
	}
	SetHandleInformation(child_out_read, HANDLE_FLAG_INHERIT, 0);
	return 0;
}

/* -------WINDOWS------
* Opens a pipe between a file and a child processes stdin.
* Return: Error code, 0 if success
*/
static int open_input_pipe(wchar_t *redirectIn){
	if (debug_global){ wprintf(L"OPEN_INPUT_PIPE: Opening input pipe...\n"); }
	if (!CreatePipe(&child_in_read, &child_in_write, &sa, 0)){
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create stdin pipe.\n");
		return 3;
	}
	else {
		if (debug_global){ wprintf(L"CREATE_PROCESS: Stdin pipe created.\n"); }
	}

	SetHandleInformation(child_in_write, HANDLE_FLAG_INHERIT, 0);

	inputFile = CreateFile(redirectIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (inputFile == INVALID_HANDLE_VALUE) {
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create handle to input file '%ws'. Does it exist?\n", redirectIn);
		return 3;
	}
	else {
		if (debug_global) wprintf(L"CREATE_PROCESS: Handle to file '%ws' created.\n", redirectIn);
	}

	return 0;
}

/* -------WINDOWS------
* Creates a properly formatted argv string
* Parameters: line->params
* Return: wchar_t representation of the argv
*/
wchar_t *get_argv(wchar_t *params, wchar_t *command){
	wchar_t *argv;

	if (params){
		if (debug_global) wprintf(L"GET_ARGV: There are parameters. Adding them to the argv.\n");
		argv = concat_string(command, L" ", params);
	}
	else {
		argv = concat_string(command, L"", NULL); //to make freeing memory easier
	}

	if (debug_global) wprintf(L"GET_ARGV: Argv is %s.\n", argv);
	return argv;
}

/* -------WINDOWS------
* Goes through the process of opening handles, creating a process and handling the pipes.
* Parameters: command_line struct
* Return: Error code, 0 if success, 50 if encountered a redirection error
*/
int create_process(command_line *line) {
	wchar_t *process_command;
	wchar_t *process_params;
	wchar_t *process_command_ext;
	wchar_t *process_params_ext;
	wchar_t  *path_commands = get_commands_dir(); // PATH/commands/
	wchar_t  *system_dir = get_system_dir(); // /bin in linux, C:/windows/system32 in windows
	wchar_t  *dirs[NUM_DIRS] = { path_commands, system_dir, L"./" };
	child_out_read = NULL;
	child_out_write = NULL;
	child_in_read = NULL;
	child_in_write = NULL;
	inputFile = NULL;
	int pError = 0;
	int rError = 0;
	int i = 0;

	// Security Attributes
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (debug_global){ wprintf(L"\nCREATE_PROCESS: Parent process ID %u\n", GetCurrentProcessId()); }
	if (debug_global) { wprintf(L"CREATE_PROCESS: Parent thread ID: %u\n", GetCurrentThreadId()); }
	if (debug_global){ display_info(line); }

	// Opening pipes
	if (line->redirectOut){
		rError = open_output_pipe();
		if (rError != 0){
			return 50;
		}
	}

	if (line->redirectIn){
		rError = open_input_pipe(line->redirectIn);
		if (rError != 0){
			return 50;
		}
	}

	/* Processing a relative path */
	if (line->type == 0) {
		// Check for the desired command in all dirs until found, also check with the extension
		while (i != NUM_DIRS){
			// Add dir to the front of the command
			process_command = concat_string(dirs[i++], line->command, NULL);

			// Create argv string and set it to params
			process_params = get_argv(line->params, process_command);

			// Start the process
			pError = create_child(process_command, process_params);

			// No errors? Get out of the loop
			if (pError == 0) {
				break;
			}
			// Errors
			else {
				if (debug_global) wprintf(L"CREATE_PROCESS: Unable to create process error %i\n", pError);
				if (debug_global) wprintf(L"CREATE_PROCESS: Trying again with extension on the end\n");
				process_command_ext = get_command_ext(process_command);

				// Create new argv string with new command and set it to params
				process_params_ext = get_argv(line->params, process_command_ext);
				pError = create_child(process_command_ext, process_params_ext);

				// No errors? Get out of the loop
				if (pError == 0) {
					break;
				}

			}
		}

	}

	/* Processing an absolute path */
	if (line->type == 1){
		if (debug_global > 1) wprintf(L"CREATE_PROCESS: Processing an absolute path\n");
		// Create argv string and set it to params
		process_params = get_argv(line->params, line->command);
		pError = create_child(line->command, process_params);
	}

	// Don't go try redirecting things if the process wasn't created succesfully, just get out of there
	if (pError != 0){
		if (debug_global) wprintf(L"CREATE_PROCESS: Unable to create process error %i. Returning\n", pError);
		close_handles();
		return pError;
	}

	// Write to the childs input buffer, it'll pick it up
	if (line->redirectIn){
		rError = write_to_pipe(inputFile);

		if (rError != 0 && rError != 109){
			pError = 50;
		}
	}

	// Read from the childs output buffer and write to file
	if (line->redirectOut){
		wchar_t *tempBuf[BUFSIZE];
		rError = read_from_pipe(line->redirectOut, &tempBuf);
		line->output = emalloc(sizeof(wchar_t)* wcslen(tempBuf));
		wcscpy(line->output, tempBuf);

		if (rError != 0 && rError != 109){
			pError = 50;
		}
	}

	// Cleaning up
	close_handles();
	if (line->type == 0){
		free(process_command);
		free(process_params_ext);
	}
	free(process_params);
	free(path_commands);
	free(system_dir);

	return pError;
}

/* -------WINDOWS------
* Actually spawns a process. Helper for createprocess.
* Parameters: command, args
* Return: Error code, 0 if success.
*/
int create_child(wchar_t *command, wchar_t *params){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL success = FALSE;
	int error = 0;

	ZeroMemory(&pi, sizeof(pi));

	// Startup info
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (child_in_read){
		si.hStdInput = child_in_read;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	if (child_out_write){
		si.hStdOutput = child_out_write;
		si.hStdError = child_out_write;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	// Spawn process
	success = CreateProcess(command, params, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (debug_global){ wprintf(L"CREATE_CHILD: Creating process in Windows %s with parameter %s\n", command, params); }
	if (!success){
		error = GetLastError();
	}
	else {
		WaitForSingleObject(pi.hProcess, INFINITE);
		if (debug_global) { wprintf(L"CREATE_CHILD: Child process ID: %u\n", GetCurrentProcessId()); }
		if (debug_global) { wprintf(L"CREATE_CHILD: Child thread ID: %u\n", GetCurrentThreadId()); }
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return error;
}

static void close_handles(){

	if (!CloseHandle(child_in_write)) {
		if (debug_global > 1) wprintf(L"CLEAN_UP: Error %u when closing child input write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child input write pipe handle closed.\n");
	}

	if (!CloseHandle(child_in_read)) {
		if (debug_global > 1) wprintf(L"CLEAN_UP: Error %u when closing child input read handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child input read pipe handle closed.\n");
	}

	if (!CloseHandle(child_out_read)) {
		if (debug_global > 1) wprintf(L"CLEAN_UP: Error %u when closing child output read handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child output read pipe handle closed.\n");
	}
}