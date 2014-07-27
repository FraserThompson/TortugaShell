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
extern void clearScreen();
extern void clearLine(int, int, int, WORD);

#endif