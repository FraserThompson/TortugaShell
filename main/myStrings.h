/* myStrings.h
* Header file for myStrings.c
*/

#ifndef MYSTRINGS
#define MYSTRINGS

extern wchar_t *convert_to_wchar(char *);
extern char *convert_to_char(wchar_t *);
extern char **split(char *, char *, int *);
extern char *concat_string(char *, char *, char*);

#endif