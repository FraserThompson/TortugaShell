/*
 * touch.c
 * Created 08/05/14
 * Author J Faulkner
 */


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	FILE* createdfile=NULL;
	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Creates a File with the name.\n");
		return EXIT_SUCCESS;
	}

	createdfile = fopen(argv[0], "rw+");

	printf("File %s has been created.", argv[0]);
	fclose(createdfile);

	return 0;

}