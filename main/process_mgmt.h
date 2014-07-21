/* process_mgmt.h
* Header file for process_mgmt.c
*/

#ifndef PROCESS_MGMT
#define PROCESS_MGMT
#include "shell.h"

extern int create_process(command_line);
extern int create_child(command_line);
extern int write_to_pipe(HANDLE);
extern int read_from_pipe(wchar_t *);
extern int open_output_pipe();
extern int open_input_pipe(wchar_t *);
extern void clean_up(command_line);

#endif
