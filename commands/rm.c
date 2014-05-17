/*
* rm.c
* Created 16/05/14
* Author J Faulkner
*/


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
	int i = 0;
	FILE* createdfile = NULL;
	int status;


	//Help message, printed by default if no arguments
	while (argv){
		printf("%s\n", argv[i]);
		if (argc == 1 || ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0))) {
			printf("RM: Permanently deletes a file. Usage: rm [file]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	status = remove(argv[0]);

	if (status == 0){
		printf("Success! File %s has been deleted\n", argv[0]);
	}
	else
	{
		printf("File delete failed. :(\n");

	}		
}
