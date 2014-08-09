/* console.h
* Header file for console.c
*/

#ifndef CONSOLE
#define CONSOLE
#include <Windows.h>

extern COORD getCursor(HANDLE);
extern COORD moveCursor(int, int, int, int, HANDLE);
extern int getConsoleWidth(HANDLE);
extern int getConsoleHeight(HANDLE);
extern int getConsoleTop(HANDLE);
extern int getConsoleBottom(HANDLE);
extern void clearLine(int, int, int, WORD);
extern void clearScreen(HANDLE);
extern void advPrint(wchar_t *, HANDLE, int, int, WORD);
extern void setTransparency(int);

#endif