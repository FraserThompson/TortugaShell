/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*
*  Returns the path to the current working directory in Windows.
*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_PATH 4096
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "../process_mgmt.h"
#include "../parser.h"
#include "../cwd.h"

/* -------LINUX------
* Returns the CWD in Linux
*/
char *getCWD(){
    char *buf = malloc(sizeof(char) * MAX_PATH);
    if (getcwd(buf, MAX_PATH) == NULL){
        return EXIT_FAILURE;
    }
    if (debug_global){ printf("GETCWD: Returning %s\n", buf); }
    return buf;
}