/* process_mgmt.h
* Header file for process_mgmt.c
*/

#ifndef PROCESS_MGMT
#define PROCESS_MGMT

extern int create_process(char *, char *, char*, char*);
extern char *get_system_dir(void);
extern char *get_commands_dir(void);
extern char *get_command_ext(char *);
extern int get_command_type(char *);
extern wchar_t *convert_to_wchar(char *);
extern char *convert_to_char(wchar_t *);
extern int write_to_pipe(char *);
extern char* read_from_pipe();

#endif
