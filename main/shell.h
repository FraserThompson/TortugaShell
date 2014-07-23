/* shell.h
* Header file for shell.c.
*/
#ifndef SHELL
#define SHELL

typedef struct command_line {
	wchar_t  *command; //command
	wchar_t  *params; //params
	wchar_t  *redirectOut; //location to redirect output to, NULL if no redirection
	wchar_t  *redirectIn; //location to redirect output from, NULL if no redirection
	wchar_t  *pipe; //location to pipe output to, NULL if no redirection
	int type; //type: 0 if relative, 1 if absolute
}command_line;

extern int debug_global;
extern wchar_t *PATH;
extern void *emalloc(size_t);
extern void *erealloc(void *, size_t);
extern wchar_t **readline(int);

#endif