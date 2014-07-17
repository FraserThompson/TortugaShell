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
#include "parser.h"
#define BUFSIZE 4096
#define NUM_DIRS 3

HANDLE child_out_read = NULL;
HANDLE child_out_write = NULL;
HANDLE child_in_read = NULL;
HANDLE child_in_write = NULL;
HANDLE inputFile = NULL;
SECURITY_ATTRIBUTES sa;

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
			fwprintf(stderr, L"WRITE_TO_PIPE: Error %u when reading file.\n", error);
			break;
		}

		chBuf[dwRead] = NULL; //add terminating character
		if (debug_global) wprintf("WRITE_TO_PIPE: Writing %s to pipe.\n", chBuf);

		if (!WriteFile(child_in_write, chBuf, dwRead, &dwWritten, NULL)) {
			error = GetLastError();
			fwprintf(stderr, L"WRITE_TO_PIPE: Error %u when writing file to pipe.\n", error);
			break;
		}
	}

	if (!CloseHandle(child_in_write)){
		fwprintf(stderr, L"WRITE_TO_PIPE: Error closing pipe.\n");
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
			chBuf[dwRead] = NULL;
			if (debug_global){ wprintf(L"READ_FROM_PIPE: Successfully read '%s' from childs standard output pipe.\n", chBuf); }
		}
		success = WriteFile(parent_out, chBuf, dwRead, &dwWritten, NULL);
		if (!success) {
			error = GetLastError();
			fwprintf(stderr, L"READ_FROM_PIPE: Error %u when writing to output pipe\n", error);
			break;
		}
		else {
			if (debug_global){ wprintf(L"READ_FROM_PIPE: Succesfully wrote '%s' to output pipe\n", chBuf); }
		}
	}

	if (!CloseHandle(parent_out)){
		fwprintf(stderr, L"READ_FROM_PIPE: Error %u when closing output handle.", GetLastError());
	}
	else {
		if (debug_global){ wprintf(L"READ_FROM_PIPE: Output pipe handle closed.\n"); }
	}

	return error;
}

/* -------WINDOWS------
* Opens a pipe between a child processes stdout and a parent process.
* Return: Error code, 0 if success
*/
int open_output_pipe(){
	if (debug_global){ wprintf(L"OPEN_OUTPUT_PIPE: Opening output pipe...\n"); }
	if (!CreatePipe(&child_out_read, &child_out_write, &sa, 0)){
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create stdout pipe.\n");
		return 3;
	}
	else {
		if (debug_global){ wprintf(L"CREATE_PROCESS: Stdout pipe created.\n"); }
	}
	SetHandleInformation(child_out_read, HANDLE_FLAG_INHERIT, 0);
}

/* -------WINDOWS------
* Opens a pipe between a file and a child processes stdin.
* Return: Error code, 0 if success
*/
int open_input_pipe(wchar_t redirectIn){
	if (debug_global){ wprintf(L"OPEN_INPUT_PIPE: Opening input pipe...\n"); }
	if (!CreatePipe(&child_in_read, &child_in_write, &sa, 0)){
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create stdin pipe.\n");
		return 50;
	}
	else {
		if (debug_global){ wprintf(L"CREATE_PROCESS: Stdin pipe created.\n"); }
	}
	SetHandleInformation(child_in_write, HANDLE_FLAG_INHERIT, 0);

	inputFile = CreateFile(redirectIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (inputFile == INVALID_HANDLE_VALUE) {
		fwprintf(stderr, L"CREATE_PROCESS: Failed to create handle to input file '%ws'. Does it exist?\n", redirectIn);
		return 50;
	}
	else {
		if (debug_global) wprintf(L"CREATE_PROCESS: Handle to file '%ws' created.\n", redirectIn);
	}
}


/* -------WINDOWS------
* Goes through the process of opening handles, creating a process and handling the pipes.
* Parameters: command_line struct
* Return: Error code, 0 if success, 50 if encountered a redirection error
*/
int create_process(command_line line) {
	wchar_t  *param_orig = line.params; //untouched params
	wchar_t  *command_orig = line.command; //untouched command
	wchar_t  *command_dir; // Command with dir on front
	wchar_t  *argv; // String list of args
	wchar_t  *command_ext; // Command with ext on end
	wchar_t  *path_commands = get_commands_dir(); // PATH/commands/
	wchar_t  *system_dir = get_system_dir(); // /bin in linux, C:/windows/system32 in windows
	wchar_t  *dirs[NUM_DIRS] = { path_commands, system_dir, L"./" };
	int pError = 0;
	int rError = 0;
	int error = 0;
	int i = 0;

	// Security Attributes
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (debug_global){ wprintf(L"\nCREATE_PROCESS: Parent process ID %u\n", GetCurrentProcessId()); }
	if (debug_global) { wprintf(L"CREATE_PROCESS: Parent thread ID: %u\n", GetCurrentThreadId()); }
	if (debug_global > 1){ display_info(line); }

	// Opening pipes
	if (line.redirectOut){
		open_output_pipe();
	}

	if (line.redirectIn){
		open_input_pipe(line.redirectIn);
	}

	/* Processing a relative path */
	if (line.type == 0) {
		// Check for the desired command in all dirs until found, also check with the extension
		while (i != NUM_DIRS){
			command_dir = concat_string(dirs[i], command_orig, NULL);
			line.params = command_dir; // First argument is always the path to the file
			line.command = command_dir;
			i++;
			// If there's a parameter add it on to the argv string
			if (param_orig){
				if (debug_global) wprintf(L"PARSE_COMMAND: There are parameters. Adding them to the argv.\n");
				line.params = concat_string(line.params, " ", param_orig);
			}

			error = create_child(line);

			// No errors
			if (error == 0) {
				return;
			}
			// Errors
			else {
				if (error == 50){
					fwprintf(stderr, L"PARSE_COMMAND: Redirect location is not accessible or does not exist.\n");
					return;
				}
				if (debug_global) wprintf(L"PARSE_COMMAND: Unable to create process error %i\n", error);
				if (debug_global) wprintf(L"PARSE_COMMAND: Trying again with extension on the end\n");
				command_ext = get_command_ext(command_dir);
				line.params = command_ext;
				line.command = command_ext;

				// If there's a parameter add it on to the argv string
				if (param_orig){
					if (debug_global) wprintf(L"PARSE_COMMAND: There are parameters. Adding them to the argv.\n");
					line.params = concat_string(line.params, L" ", param_orig);
				}

				error = create_child(line);

				// No errors
				if (error == 0) {
					return;
				}
				// Errors
				else {
					if (debug_global){ wprintf(L"PARSE_COMMAND: Unable to create process error %i\n", error); }
				}

			}
		}

	}

	/* Processing an absolute path */
	if (line.type == 1){

		if (param_orig){
			argv = concat_string(line.command, L" ", line.params);
		}
		else {
			argv = line.command;
		}

		line.params = argv;

		error = create_child(line);

		// No errors
		if (error == 0) {
			return;
		}
		else {
			if (error == 50){
				fwprintf(stderr, L"PARSE_COMMAND: Redirect location is not accessible or does not exist.\n");
				return;
			}
		}
	}

	wprintf(L"'%s' does not exist.\n", line.command);

	// Spawn process
	//pError = create_child(command_wchar, param_wchar, line.redirectOut, line.redirectIn);

	// Don't go try redirecting things if the process wasn't created succesfully, just get out of there
	if (pError != 0){
		clean_up();
		return pError;
	}

	// Write to the childs input buffer, it'll pick it up
	if (line.redirectIn){
		rError = write_to_pipe(inputFile);
		// Don't continue if it couldn't redirect properly
		if (rError != 0 && rError != 109){
			fwprintf(stderr, L"CREATE_PROCESS: Input redirection error %i. Halting.\n", rError);
			pError = 50;
		}
	}

	// Read from the childs output buffer
	if (line.redirectOut){
		rError = read_from_pipe(line.redirectOut);
		// Don't continue if it couldn't redirect properly
		if (rError != 0 && rError != 109){
			fwprintf(stderr, L"CREATE_PROCESS: Output redirection error %i. Halting.\n", rError);
			pError = 50;
		}
	}

	//clean_up();
	return pError;
}

static int clean_up(void){

	if (!CloseHandle(child_in_write)) {
		if (debug_global > 1) wprintf(L"CLEAN_UP: Error %u when closing child input write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child input write pipe handle closed.\n"); 
	}

	if (!CloseHandle(child_out_write)) {
		if (debug_global > 1) wprintf(L"CLEAN_UPE: Error %u when closing child output write handle. Could be that it doesn't exist, that's okay.\n", GetLastError());
	}
	else {
		if (debug_global) wprintf(L"CLEAN_UP: Child output write pipe handle closed.\n");
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

/* -------WINDOWS------
* Actually spawns a process. Helper for createprocess.
* Parameters: command, args
* Return: Error code, 0 if success.
*/
int create_child(command_line line){
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL success = FALSE;
	int error = 0;

	ZeroMemory(&pi, sizeof(pi));

	// Startup info
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (line.redirectIn){
		si.hStdInput = child_in_read;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	if (line.redirectOut){
		si.hStdOutput = child_out_write;
		si.hStdError = child_out_write;
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	// Spawn process
	success = CreateProcess(line.command, line.params, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (debug_global){ wprintf(L"CREATE_CHILD: Created process in Windows %s with parameter %s\n", line.command, line.params); }
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