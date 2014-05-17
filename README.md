TORTUGA SHELL
==============
This is a simple shell designed for learning.

Main Modules
-------------
These modules contain functions called within the main loop.

####/main/shell.c:
Contains the main loop which prints an arrow and takes user input.
  
####/main/parser.c
`parse_command(char *, char *, int)`: Processes a line of commands.  
`parse(char *)`: Parses an individual command.  
`char **split(char *)`: Splits a string into an array of words.  
`char *concat_string(char *, char *, char*)`: Joins up to three strings. 
`int debug_global`: Global variable signifying whether or not to print debug text.
`char* PATH`: Global variable used to hold the path the user ran Tortuga from.

####/main/platform/process_mgmt.c
`int create_process(char *command, char *params)`: Creates a process under windows.  

####/main/platform/cd.c
`void cd(char *)`: Changes the current working directory.

####/main/platform/cwd.c
`void *getCWD(void)`: Returns the path fo the current working directory.

####/main/platform/help.c
`void print_help(void)`: Returns the name of all the executables in the ./commands directory alongside their -h call.

