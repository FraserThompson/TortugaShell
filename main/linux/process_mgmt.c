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
	return "/bin/";
}

/* -------LINUX------
* Returns the cwd with commands dir on the end.
* Return: cwd with commands dir on the end
*/
char *get_commands_dir(void){
    if (debug_global){ printf("GET_COMMANDS_DIR: Getting it for Linux.\n"); }
    return concat_string(PATH, "/commands/", NULL);
}
/* -------LINUX------
* Creates a process in Unix/Linux.
* Parameters: Location of process to spawn
* Return: Error code, 0 if success.
*/
int create_process(char *command, char *params) {
	pid_t parent = getpid();
	pid_t child = fork();
        pid_t tpid;
        int status;
	if (debug_global){ printf("CREATE_PROCESS: Creating %s in Linux with params %s.\n", command, params); }

        /* This is if something goes wrong */
	if (child == -1){
		fprintf(stderr, "CREATE_PROCESS: Unable to fork process!\n");
		return EXIT_FAILURE;
        /* This is the parent waiting for the child to finish */
	} else if (child > 0){
            wait(&status);
        /* This is the child being executed */
	} else {
		execve(command, params, NULL);
                printf("CREATE_PROCESS: Could not execute command \'%s\'!\n", command);
		return EXIT_FAILURE;
	}
	return 0;
}
