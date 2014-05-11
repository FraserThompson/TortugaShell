/* parser.h
 * Header file for parser.c.
 */
#ifndef PARSER
#define PARSER

extern void parse_command(char *, char **);
extern void parse(char *);
extern char **split(char *);
extern char *concat_string(char *, char *);

#endif
