/*
* cp.c
* Created 08/05/14
* Author J Faulkner
*/


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -----CROSS PLATFORM----
* Copies a file.
*/
int main(int argc, char *argv[]){
	char ch;
	int i = 0;
	FILE* sourcefile = NULL;
	FILE* outputfile = NULL;


	//Help message, printed by default if no arguments
	while (argv){
		printf("%s\n", argv[i]);
		if (argc == 1 || ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0))) {
			printf("CP: Copies the contents of the requested file. Usage: cat [file1] [file2]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	sourcefile = fopen(argv[0], "r");
	outputfile = fopen(argv[1], "w+");

	while ((ch = fgetc(sourcefile)) != EOF){
	fputc(ch, outputfile);	
	}
	fclose(sourcefile);
	fclose(outputfile);
}
