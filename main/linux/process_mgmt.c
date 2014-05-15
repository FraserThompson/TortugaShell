/*
* process_mgmt.c
*
*  Created on: 11/05/2014
*      Author: Fraser
*  
*  Contains methods which help with creating processes on Linux.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../parser.h"


/* -------LINUX------
* Returns the path to the system dir.
* Return: path to the system dir
*/
char *get_system_dir(void){
	if (debug_global){ printf("GET_SYSTEM_DIR: Getting it for Linux.\n"); }
	return "/bin";
}

/* -------LINUX------
* Creates a process in Unix/Linux.
* Parameters: Location of process to spawn
* Return: Error code, 0 if success.
*/
int create_process(char *command, char *params) {
	pid_t parent = getpid();
	pid_t child = fork();
	if (debug_global){ printf("CREATE_PROCESS: Creating %s in Linux with params %s.\n", command, params); }

	if (child == -1){
		fprintf(stderr, "CREATE_PROCESS: Unable to fork process!");
		exit(EXIT_FAILURE);
	} else if (child > 0){
		int status;
		waitpid(child, &status, 0);
	} else {
		execve(command, params, NULL);
		exit(EXIT_FAILURE);
	}
	return 0;
}
