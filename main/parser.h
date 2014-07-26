/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER
#include "shell.h" //for the command_line struct

extern void parse(wchar_t **, int);
extern void display_info(command_line *);
extern wchar_t  *get_system_dir(void);
extern wchar_t  *get_commands_dir(void);
extern wchar_t  *get_command_ext(wchar_t *);
extern int get_command_type(wchar_t *);
extern void free_stuff(wchar_t **, int);

#endif
