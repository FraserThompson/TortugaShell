TORTUGA SHELL
==============
This is a simple shell designed for learning.

Main Modules
-------------
These modules contain functions called within the main loop.

#####Cross-platform
######/main/shell.c
`static char *readline(void)`: Prints a prompt containing the CWD and reads user input.  
  
######/main/parser.c
`void parse_command(command_line`:  
Processes an individual command.  
Parameters: The command_line struct.
`void parse(char *)`:  
Parses a line of commands.  
Parameter: Line of commands.  
`char **split(char *, char *, int *)`:  
Splits a string into an array of words.  
Parameters: String, delimiter, address of variable to hold index of last item.  
`char *concat_string(char *, char *, char*)`:  
Joins up to three strings.  
Parameters: First string, second string, third string (may be null).  
`int debug_global`:  
Global variable signifying whether or not to print debug text.  0 for none, 1 for main functions, 2 for everything  
`char* PATH`:  
Global variable used to hold the path the user ran Tortuga from.  
`typedef struct command_line`:  
Contains information extracted from a user-inputted command line.  

#####Platform-specific
######/main/platform/process_mgmt.c
`int get_command_type(char *)`:  
Used internally to check whether it's a full path or a relative one.  
Parameter: String to check.  
`int create_process(char *, char *)`:  
Creates a process under platform.  
Parameters: Path to process, parameters  
`char *get_system_dir(void)`:  
Returns the path to the system directory.  
`char *get_commands_dir(void)`:  
Returns the PATH the application was run from with \\commands\\ on the end.  
Return: path  
`char *get_command_ext(char *)`:
Returns the command with the extension added (.exe in Windows).  
Parameter: Command to attach it to.  
Return: Command with extension added  
`wchar_t *convert_to_wchar(char *)`:  
Converts a char to a wchar. (windows only)  
Parameter: Char to convert.   
`char *convert_to_char(wchar_t *)`:  
Converts a wchar to a char.  
Parameter: Wchar to convert. (windows only)  

######/main/platform/cd.c
`void cd(char *)`:  
Changes the current working directory.  
Parameter: Directory to change to.  

######/main/platform/cwd.c
`char *getCWD(void)`:  
Returns the path of the current working directory.  

######/main/platform/help.c
`void print_help(void)`:  
Returns the name of all the executables in the ./commands directory alongside their -h call.  

Command Modules
-------------

These are modules which are compiled into seperate executables in the ./commands directory to be called as processes. 
