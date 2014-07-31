/* shell.h
* Header file for shell.c.
*/
#ifndef SHELL
#define SHELL
#include <Windows.h>

typedef struct command_line {
	wchar_t  *command; //command
	wchar_t  *params; //params
	wchar_t *output; //storing process output
	wchar_t  *redirectOut; //location to redirect output to, NULL if no redirection
	wchar_t  *redirectIn; //location to redirect output from, NULL if no redirection
	wchar_t  *pipe; //location to pipe output to, NULL if no redirection
	int type; //type: 0 if relative, 1 if absolute
}command_line;

// Global variables
extern int debug_global;
extern wchar_t *PATH;
extern HANDLE CONSOLE_INPUT;
extern HANDLE CONSOLE_OUTPUT;
extern int CONSOLE_TRANSPARENCY;
extern WORD HEADER_FOOTER_ATTRIBUTES;
extern WORD NORMAL_ATTRIBUTES;

// Methods
extern void *emalloc(size_t);
extern void *erealloc(void *, size_t);
extern wchar_t **readline(int);
extern void advPrint(wchar_t *, HANDLE, int, int, WORD);
extern void printHeader(wchar_t *);
extern void printFooter(wchar_t *);

#endif