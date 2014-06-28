/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER

typedef struct line_info {
	int type;
	int redirection;
	int piping;
} line_info;

extern void parse_command(char *, char *, line_info);
extern void parse(char *);
extern char **split(char *, char *, int *);
extern char *concat_string(char *, char *, char*);
extern int debug_global;
extern char *PATH;


#endif
