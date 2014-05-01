/* parser.h
 * Header file for parser.c
 */
#ifndef PARSER
#define PARSER

extern void parse_command(wchar_t *, wchar_t **);
extern void parse(wchar_t *);
extern void print_info();
extern void free_info();

#endif
