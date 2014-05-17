/*
 * touch.c
 * Created 08/05/14
 * Author J Faulkner
 */


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -----CROSS PLATFORM----
* Creates a file.
*/
int main(int argc, char *argv[]){
	FILE* createdfile=NULL;
	int i = 0;

	//Help message, printed by default if no arguments
	while (argv[i]){
		if (argc == 1 || ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0))) {
			printf("TOUCH: Creates an empty file. Usage: touch [file]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	createdfile = fopen(argv[1], "w+");

	printf("TOUCH: File %s has been created.\n", argv[1]);
	fclose(createdfile);

	return 0;

}