/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER

typedef struct command_line {
	wchar_t  *command; //command
	wchar_t  *params; //params
	wchar_t  *redirectOut; //location to redirect output to, NULL if no redirection
	wchar_t  *redirectIn; //location to redirect output from, NULL if no redirection
	wchar_t  *pipe; //location to pipe output to, NULL if no redirection
	int type; //type: 0 if relative, 1 if absolute
	
} command_line;

extern void parse_command(command_line);
extern void parse(wchar_t *);
extern void display_info(command_line);
extern wchar_t  *get_system_dir(void);
extern wchar_t  *get_commands_dir(void);
extern wchar_t  *get_command_ext(wchar_t *);
extern int get_command_type(wchar_t *);

#endif
