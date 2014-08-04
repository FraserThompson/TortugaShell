/*
* grep.c
* Created 16/05/14
* J Faulkner
* FOUND AT:
* http://cm.bell-labs.com/cm/cs/tpop/grep.c
* OTHER POSSIBLE RESOURCE:
* http://www.codingunit.com/c-tutorial-searching-for-strings-in-a-text-file
*/


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	matchhere(char*, char*);
int	matchstar(int, char*, char*);
int grep(char*, FILE*, char*);
int match(char*, char*);

int main(int argc, char *argv[]){
	int i = 0;
	FILE *f= NULL;

	//Help message, printed by default if no arguments
	while (argv[i]){
		if (argc == 1 || ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-h") == 0))) {
			printf("grep\tSearches a file for a string.\n\tUsage: grep [file] [search string]\n");
			return EXIT_SUCCESS;
		}
		i++;
	}

	if (argv[0]){
		f = fopen(argv[0], "r");

		grep(argv[1], f, NULL);
	}
}
int grep(char *regexp, FILE *f, char *name)
{
	int n, nmatch;
	char buf[BUFSIZ];

	nmatch = 0;
	while (fgets(buf, sizeof buf, f) != NULL) {
		n = strlen(buf);
		if (n > 0 && buf[n - 1] == '\n')
			buf[n - 1] = '\0';
		if (match(regexp, buf)) {
			nmatch++;
			if (name != NULL)
				printf("%s:", name);
			printf("%s\n", buf);
		}
	}
	return nmatch;
}

int match(char *regexp, char *text)
{
	if (regexp[0] == '^')
		return matchhere(regexp + 1, text);
	do {	/* must look even if string is empty */
		if (matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}


int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp + 2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';
	if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
		return matchhere(regexp + 1, text + 1);
	return 0;
}

int matchstar(int c, char *regexp, char *text)
{
	do {	/* a * matches zero or more instances */
		if (matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));
	return 0;
}

