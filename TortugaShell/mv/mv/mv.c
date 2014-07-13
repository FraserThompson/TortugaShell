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
	int fd1, fd2, count;
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
		exit(1);
	}
	else{
		fd1 = open(argv[1], O_RDONLY);
		if (fd1 == -1){
			printf("File does not exist.");
			exit(1);
		}
		fd2 = open(argv[2], O_WRONLY);
		if (fd2 == -1){
			fd2 = creat(argv[2], 0666);
		}
		while ((count = read(fd1, buf, 512)) > 0){
			write(fd2, buf, count);
		}
		close(fd2);
		close(fd1);
	}
	return EXIT_SUCCESS;
}
