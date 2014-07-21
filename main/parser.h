/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER
#include "shell.h"

extern void parse_command(command_line);
extern void parse(wchar_t *);
extern void display_info(command_line);
extern wchar_t  *get_system_dir(void);
extern wchar_t  *get_commands_dir(void);
extern wchar_t  *get_command_ext(wchar_t *);
extern int get_command_type(wchar_t *);
extern void free_stuff(wchar_t **, int);

#endif
