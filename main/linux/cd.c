/*
* cd.c
*
*  Created on: 17/05/2014
*      Author: Fraser
*
*  Changes the current working directory.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../parser.h"
#include "../cd.h"

/* -----LINUX-------
* Changes directory.
*/
void cd(char *dir) {
    if (debug_global){ printf("CD: Changing directory to %s\n", dir); }
    if (chdir(dir) == 0){
        return;
    }
    printf("CD: Error changing directory.\n");
    return;
}

