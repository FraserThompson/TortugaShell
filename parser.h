/* parser.h
 * Header file for parser.c
 */
#ifndef PARSER
#define PARSER

typedef struct {
	char *name;
	char *usage;
	char *params;
	int type;
} parseInfo;

extern void parse_command(char);
extern void *parse(char);
extern void print_info();
extern void free_info();

#endif
