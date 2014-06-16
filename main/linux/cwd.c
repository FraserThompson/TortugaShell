/*
* cwd.c
*
*  Created on: 6/05/2014
*      Author: Fraser
*
*  Returns the path to the current working directory in Windows.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "../process_mgmt.h"
#include "../cwd.h"

/* -------LINUX------
* Returns the CWD in Linux
*/
char *getCWD(){
	return "/bin";
}