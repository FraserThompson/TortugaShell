/* parser.h
 * Header file for parser.c
 */
#ifndef PARSER
#define PARSER

struct parseInfo;
void init_info(parseInfo);
void parse_command(char, struct);
parseInfo *parse (char);
void print_info (parseInfo);
void free_info (parseInfo);

#endif
