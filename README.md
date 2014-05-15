TORTUGA SHELL
==============
This will eventually be a shell. This documentation should be updated as things are added.

Main Modules
-------------
####/main/shell.c:
Contains the main loop which prints an arrow and takes user input.
  
####/main/parser.c
`parse_command(char *, char *, int)`: Processes a line of commands.  
`parse(char *)`: Parses an individual command.  
`char **split(char *)`: Splits a string into an array of words.  
`char *concat_string(char *, char *, char*)`: Joins up to three strings.  

####/main/platform/process_mgmt.c
`int create_process_unix(char *command, char *params)`: Creates a process under unix/linux.  
`int create_process_win(char *command, char *params)`: Creates a process under windows.  
