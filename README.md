~~~~~~~~~~~~TORTUGA SHELL~~~~~~~~~

This will eventually be a shell. This documentation should be updated as things are added.



parser.h:
  
	init_info(parseInfo *p)
    
		Initialize struct:
          
			Name
          
			Usage
          
			Type
          
  
	parse_command(char * command, struct commandType *comm)
    
		Perform syntax highlighting, call command's associated c file
  
	parseInfo *parse (char *cmdline)
    
		Parsing multiple commands, redirection, piping
  
	print_info (parseInfo *info)
  
	free_info (parseInfo *info)
  


parser.c:



shell.h:



shell.c:



commands.h:
  
	echo
  
	...
  

/commands/:
  
	echo.c
  
	...
