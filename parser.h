/* parser.h
 * Header file for parser.c
 */
#ifndef PARSER
#define PARSER

typedef struct {
		char *name;
		char *usage;
		int type;
} parseInfo;

extern void parse_command (char, parseInfo);
extern void *parse (char);
extern void print_info (parseInfo);
extern void free_info (parseInfo);

#endif
