/*
* cp.c
* Created 08/05/14
* Author J Faulkner
*/


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
	char ch;
	FILE* sourcefile = NULL;
	FILE* outputfile = NULL;

	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Copies a file.\n");
		return EXIT_SUCCESS;
	}

	sourcefile = fopen(argv[0], "r");
	outputfile = fopen(argv[1], "w+");

	while ((ch = fgetc(sourcefile)) != EOF){
	fputc(ch, outputfile);	
	}
	fclose(sourcefile);
	fclose(outputfile);
}
