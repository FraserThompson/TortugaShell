/*
* help.c
*
*  Created on: 16/05/2014
*      Author: Fraser
* Goes through the ./commands directory printing the name of each file and the -h text
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include "help.h"
#include "myStrings.h"
#include "process_mgmt.h"
#include "shell.h"

/* -----WINDOWS----
* Searches the ./commands directory and prints each filename, and then calls the file with -h to get the help.
*/
void get_help(void){

	// First add the hardcoded built in commands
	wchar_t *built_in = L"\n\t----------BUILT IN COMMANDS----------\n\ncd\tChanges the current working directory.\n\tUsage: cd [directory]\ncwd\tPrints the current working directory.\n\tUsage: cwd\nhelp\tPrints this help message.\n\tUsage: help\n";

	// Then work on the binaries in the commands subdirectory
	wchar_t *externcomm = L"\n\t----------EXTERNAL COMMANDS----------\n";

	inorder(command_tree);
	//wchar_t *result = concat_string(built_in, externcomm, extern_result);
}