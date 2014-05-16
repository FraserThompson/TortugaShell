/*
* cat.c
* Created 14/05/14
* Author: LeYing Tran
* Copies the contents of the given file
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
	FILE *fp;
	if ((strcmp(argv[0], "-help") == 0) || (strcmp(argv[0], "-h") == 0)) {
		printf("Copies the contents of the requested file.\n");
		return EXIT_SUCCESS;
	}
	
	if ((fp = fopen(argv[0], "r")) != NULL){
		char line[256];
		while (fgets(line, 256, fp)){
			fputs(line, stdout);
		}
		fclose(fp);
		exit(1);
	}
	else
	{
		printf("Error opening file.\n");
		exit(1);
	}
}
