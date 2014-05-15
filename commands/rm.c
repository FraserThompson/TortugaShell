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

	FILE* createdfile = NULL;
	int status;

	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Deletes a File with the name given.\n");
		return EXIT_SUCCESS;
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
