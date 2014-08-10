/* myStrings.h
* Header file for myStrings.c
*/

#ifndef MYSTRINGS
#define MYSTRINGS

extern wchar_t *convert_to_wchar(char *);
extern char *convert_to_char(wchar_t *);
extern wchar_t **split(wchar_t *, wchar_t *, int *);
extern wchar_t *concat_string(wchar_t *, wchar_t *, wchar_t*);

#endif