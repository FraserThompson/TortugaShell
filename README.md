TORTUGA SHELL
==============
This is a simple shell designed for learning.

Main Modules
-------------
These modules contain functions called within the main loop.

####/main/shell.c:
`static char *readline(void)`: Prints a prompt containing the CWD and reads user input.  
  
####/main/parser.c
`static int command_type(char *)`: Used internally to check whether it's a full path or a relative one.  Parameter: String to check.  
`parse_command(char *, char *, int)`: Processes an individual command. Parameters: Command, parameter, type.  
`parse(char *)`: Parses a line of commands.  Parameter: Line of commands.  
`char **split(char *, char *, int *)`: Splits a string into an array of words. Parameters: String, delimiter, address of variable to hold index of last item.  
`char *concat_string(char *, char *, char*)`: Joins up to three strings. Parameters: First string, second string, third string (may be null).  
`int debug_global`: Global variable signifying whether or not to print debug text.  
`char* PATH`: Global variable used to hold the path the user ran Tortuga from.  

####/main/platform/process_mgmt.c
`int create_process(char *, char *)`: Creates a process under windows. Parameters: Path to process, parameters  
`char *get_system_dir(void)`: Returns the path to the system directory.  
`wchar_t *convert_to_wchar(char *)`: Converts a char to a wchar.  Parameter: Char to convert.  
`char *convert_to_char(wchar_t *)`: Converts a wchar to a char.  Parameter: Wchar to convert.  

####/main/platform/cd.c
`void cd(char *)`: Changes the current working directory. Parameter: Directory to change to.  

####/main/platform/cwd.c
`char *getCWD(void)`: Returns the path fo the current working directory.  

####/main/platform/help.c
`void print_help(void)`: Returns the name of all the executables in the ./commands directory alongside their -h call.  

Command Modules
-------------

These are modules which are compiled into seperate executables in the ./commands directory to be called as processes. 
