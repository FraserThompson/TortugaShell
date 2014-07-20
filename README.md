TORTUGA SHELL
==============
This is a simple shell designed for learning.

Main Modules
-------------
These modules contain functions called within the main loop.

######/main/shell.c
`static char *readline(void)`: Prints a prompt containing the CWD and reads user input.  
`int debug_global`:  
Global variable signifying whether or not to print debug text.  0 for none, 1 for main functions, 2 for everything  
`wchar_t* PATH`:  
Global variable used to hold the path the user ran Tortuga from.  
`typedef struct command_line`:  
Contains information extracted from a user-inputted command line.  
`void *emalloc(size_t)`:  
Malloc with error checking.  
`void *erealloc(void *, size_t)`:  
Realloc with error checking.
  
######/main/parser.c
`void parse_command(command_line)`:  
Processes an individual command.  
Parameters: The command_line struct.
`void parse(wchar_t *)`:  
Parses a line of commands.  
Parameter: Line of commands.  
`void display_info(command_line)`:  
Prints out everything contained in the struct.  
Parameter: command_line struct  
`wchar_t *get_system_dir(void)`:  
Returns the path to the system directory.  
`wchar_t *get_commands_dir(void)`:  
Returns the PATH the application was run from with \\commands\\ on the end.  
Return: path  
`wchar_t *get_command_ext(wchar_t *)`:  
Returns the command with the extension added (.exe in Windows).  
Parameter: Command to attach it to.  
Return: Command with extension added  
`int get_command_type(wchar_t *)`:  
Used internally to check whether it's a full path or a relative one.  
Parameter: String to check.  

######/main/myStrings.h  
`wchar_t **split(wchar_t *, wchar_t *, int *)`:  
Splits a string into an array of words.  
Parameters: String, delimiter, address of variable to hold index of last item.  
`wchar_t *concat_string(wchar_t *, wchar_t *, wchar_t *)`:  
Joins up to three strings.  
Parameters: First string, second string, third string (may be null).  
`wchar_t *convert_to_wchar(char *)`:  
Converts a char to a wchar.
Parameter: Char to convert.   
`char *convert_to_char(wchar_t *)`:  
Converts a wchar to a char.  
Parameter: Wchar to convert. 

######/main/process_mgmt.c
`int create_process(char *, char *)`:  
Goes through the process of opening handles, creating a process and handling the pipes.  
Parameters: command_line struct  
Return: Error code, 0 if success, 50 if encountered a redirection error  
`int create_child(command_line)`:  
Actually spawns a process. Helper for createprocess.  
Parameters: command, args  
Return: Error code, 0 if success.  
`int write_to_pipe(HANDLE)`:  
Writes to a child processes pipe  
Parameters: Location of file to put into the pipe  
Return: Error code - 0 if success.  
`int read_from_pipe(wchar_t *)`:  
Reads from a child processes pipe and write to a specified file  
Parameters: Location to write to  
Return: Error code - 0 if success  
`int open_output_pipe()`:  
Opens a pipe between a child processes stdout and a parent process.  
Return: Error code, 0 if success  
`int open_input_pipe(wchar_t *)`: 
Opens a pipe between a file and a child processes stdin.  
Return: Error code, 0 if success  
`void clean_up()`:  
Closes unused handles so they can be freed up for other processes  
`wchar_t *get_argv(wchar_t *params, wchar_t *command)`:  
Creates a properly formatted argv string  
Parameters: line.params  
Return: wchar_t representation of the argv  


######/main/cd.c
`void cd(wchar_t *)`:  
Changes the current working directory.  
Parameter: Directory to change to.  

######/main/cwd.c
`wchar_t *getCWD(void)`:  
Returns the path of the current working directory.  

######/main/help.c
`void print_help(void)`:  
Returns the name of all the executables in the ./commands directory alongside their -h call.  

Command Modules
-------------

These are modules which are compiled into seperate executables in the ./commands directory to be called as processes. 
