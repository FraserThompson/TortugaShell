/* process_mgmt.h
* Header file for process_mgmt.c
*/

#ifndef PROCESS_MGMT
#define PROCESS_MGMT

extern int create_process(command_line);
extern int write_to_pipe(HANDLE);
extern int read_from_pipe(char *);

#endif
