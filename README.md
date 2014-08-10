TORTUGA SHELL
==============
This is a simple shell designed for learning.

Execution Path
-------------
 First build_command_tree() is called to build a BST from the ./commands directory to allow for syntax highlighting.
drawPrompt() then draws the prompt and header.  
readline() begins processing characters inputted by the user and provides backspace/arrow key functionality. highlightCommand() is called every time a character is inputted to check to see if it's a known command and print the associated help message, and to check if it's an existing directory/file. Every time a slash is inputted it builds a BST from the directory to facilitate quick searching.
Once the user presses enter the line is given to split() to split into an array of tokens by spaces and quotation marks. This array is given to Parse() where it is processed into a command_line structure containing the command, arguments, and redirection locations if given. Parse sends the struct to create_process() who opens the relevant pipes (if redirection is specified) and then creates the child process by calling create_child(). 
Once the execution is finished create_process() cleans up and returns an error code specifying whether it was successfull. The main loop continues until parse() returns 0 indicating the user wishes to exit.

Main Modules
-------------
These modules contain functions called within the main loop.

######/main/shell.h
Defines methods for displaying the shell interface and handling user input.  

######/main/parser.
Defines methods which deal with parsing user input taken by methods in shell.h.  

######/main/console.h
Defines methods for dealing with things specific to the Windows Console such as cursor movement.  

######/main/gui.h
Defines methods which are used when displaying the settings GUI.  

######/main/myStrings.h
Defines methods which do stuff to strings.  

######/main/process_mgmt.
Defines methods which deal with executing processes on Windows.  

######/main/cd.h
Defines a method which changes the current working directory on Windows.  

######/main/cwd.h
Defines a method which prints the current working directory on Windows.  

######/main/help.h
Defines a method which prints help for internal/external commands.  

######/main/bst.h
Defines the binary search tree structure and methods.


Command Modules
-------------

These are modules which are compiled into seperate executables in the ./commands directory to be called as processes. Each module should print a string when called with -h.
