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

	sourcefile = fopen(argv[0], "r");
	outputfile = fopen(argv[1], "w+");

	while ((ch = fgetc(sourcefile)) != EOF){
	fputc(ch, outputfile);	
	}
	fclose(sourcefile);
	fclose(outputfile);
}
