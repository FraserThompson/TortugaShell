/*
* mv.c
* Created 12/07/14
* Author LeYing Tran
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/* 
* Moves a file from one location to another.
*/
int main(int argc, char *argv[]){
	int i = 0;
	FILE *fd1 = NULL;
	FILE *fd2 = NULL;
	int count = 0;
	char buf[512];

	//Help message, printed by default if no arguments
	while (argv[i]){
		if (argc == 1 || ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0))) {
			printf("mv\t Moves a file from one location to another.\n\tUsage: mv [filename] [desired file location]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	if (argc != 3){
		printf("Please check filename.");
		return EXIT_SUCCESS;
	}
	else{
		if (argv[1]){
			fd1 = fopen(argv[1], "r");
			if (!fd1){
				fprintf(stderr, "MV: Input file '%s' does not exist.", argv[1]);
				return EXIT_FAILURE;
			}
			fd2 = fopen(argv[2], "w+");
			if (!fd2){
				perror("Error: ");
				fprintf(stderr, "MV: Could not open output file '%s' for writing.\n", argv[2]);
				return EXIT_FAILURE;
			}

			while (fgets(buf, sizeof(buf), fd1) != NULL){
				fprintf(fd2, buf);
			}

			fclose(fd2);
			fclose(fd1);
		}
	}
	return EXIT_SUCCESS;
}
