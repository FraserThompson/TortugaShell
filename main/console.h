/* console.h
* Header file for console.c
*/

#ifndef CONSOLE
#define CONSOLE
#include <Windows.h>

extern COORD getCursor(void);
extern COORD moveCursor(int, int, int, int);
extern int getConsoleWidth(void);
extern int getConsoleHeight(void);

#endif