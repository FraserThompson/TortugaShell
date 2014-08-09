/* gui.h
* Header file for gui.c
*/

#ifndef GUI
#define GUI
#include <Windows.h>

extern COORD draw_options(wchar_t **, int *, int, int, COORD, int, int);
extern COORD draw_settings(WORD, WORD, wchar_t *, wchar_t *, wchar_t **, int *, int, int, int, int, int, int);
extern COORD draw_a_box(COORD, WORD, WORD, wchar_t *, wchar_t *, int, int, int, int, int);
extern COORD clear_a_box(COORD, int, int, int, int);

#endif