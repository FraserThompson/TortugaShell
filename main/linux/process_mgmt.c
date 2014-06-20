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

/* -----LINUX----
* Check to see what the command type is: Whether it's just a single command or a physical path. 
* This is Linux so it does this by checking if the first character is a /.
* Parameter: Command to check.
* Return: An integer indicating whether it's a command (0) or a path (1)
*/
int command_type(char *command){
	if (debug_global){ printf("COMMAND_TYPE: Input: %s\n", command); }
	if (command[0] == '/'){
		if (debug_global){ printf("COMMAND_TYPE: It's a path.\n"); }
		return 1;
	}
	if (debug_global){ printf("COMMAND_TYPE: It's a command.\n"); }
	return 0;
}

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
* Returns the command with the extension added (does nothing in Linux).
* Parameter: Command 
* Return: Command 
*/
char *get_command_ext(char *command){
    return command;
}

/* -------LINUX------
* Creates a process in Unix/Linux.
* Parameters: Location of process to spawn
* Return: Error code, 0 if success.
*/
int create_process(char *command, char *argv) {
	pid_t child;
        int status;
        int last;
        int i = 0;
      
        char **param_array = split(argv, " ", &last); // Linux needs argv in an array
        
	if (debug_global){ 
            printf("CREATE_PROCESS: Creating %s in Linux with params:\n", command); 
            while (i < last){
                printf("\t %s\n", param_array[i++]);
            }
        }
        child = fork();

        switch (child) {
        /* This is if something goes wrong */
        case -1:
            printf("CREATE_PROCESS: Unable to fork process!\n");
            return EXIT_FAILURE;
        /* If we're in the child process we can execute a thing */
        case 0:
            execve(command, param_array, NULL);
            // If we're still here then something is wrong
            printf("CREATE_PROCESS: Could not execute command \'%s\'!\n", command);
            return EXIT_FAILURE;
        /* This is the parent waiting for the child to finish */
        default:
            if (debug_global){ printf("CREATE_PROCESS: Child PID = %i\n", child); }
            if (wait(&status) != -1){
                if (debug_global){ printf("CREATE_PROCESS: Error! Child exited with status %i\n", status); } 
            } else {
                if (debug_global){ printf("CREATE_PROCESS: Child terminated."); } 
            }
            break;
	}
	return 0;
}
