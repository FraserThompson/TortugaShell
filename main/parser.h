/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER

typedef struct command_line {
	char *command; //command
	char *params; //params
	char *redirectOut; //location to redirect output to, NULL if no redirection
	char *redirectIn; //location to redirect output from, NULL if no redirection
	char *pipe; //location to pipe output to, NULL if no redirection
	int type; //type: 0 if relative, 1 if absolute
	
} command_line;

extern void parse_command(command_line);
extern void parse(char *);
extern void display_info(command_line);
extern char *get_system_dir(void);
extern char *get_commands_dir(void);
extern char *get_command_ext(char *);
extern int get_command_type(char *);
extern int debug_global;
extern char *PATH;


#endif
