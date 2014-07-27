#include "shell.h"

void clearScreen(){
	DWORD written;
	COORD coords = { 0, 0 };
	DWORD cells = getConsoleWidth() * getConsoleHeight();

	FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', cells, coords, &written);
	FillConsoleOutputAttribute(CONSOLE_OUTPUT, NULL, cells, coords, &written);
}

void clearLine(int width, int x, int y, WORD attributes){
	DWORD written;
	COORD coords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	coords.X = x;
	coords.Y = y;

	if (y == 0){
		GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
		coords.Y = screen_info.srWindow.Top;
	}

	FillConsoleOutputAttribute(CONSOLE_OUTPUT, attributes, width, coords, &written);
	FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width, coords, &written);
}


/* -----WINDOWS----
* Gets the current console cursor position
* Return: Cursor position
*/
COORD getCursor(){
	CONSOLE_SCREEN_BUFFER_INFO cursor_position;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &cursor_position);
	return cursor_position.dwCursorPosition;
}

/* -----WINDOWS----
* Gets the width of the console
* Return: Console width in columns
*/
int getConsoleWidth(){
	CONSOLE_SCREEN_BUFFER_INFO console;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &console);
	int width = console.dwSize.X;
	return width;
}

/* -----WINDOWS----
* Gets the height of the console
* Return: Console width in rows
*/
int getConsoleHeight(){
	CONSOLE_SCREEN_BUFFER_INFO console;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &console);
	//int height = console.dwSize.Y;
	int height = console.srWindow.Bottom - console.srWindow.Top + 1;
	return height;
}

/* -----WINDOWS----
* Moves the console cursor to a specified position relative to current
* Params: num of rows/columns to move relative or absolute
* Return: The new position
*/
COORD moveCursor(int x, int y, int x_abs, int y_abs) {
	COORD coords;
	CONSOLE_SCREEN_BUFFER_INFO cursor_position;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &cursor_position);
	coords = cursor_position.dwCursorPosition;
	if (x_abs >= 0 || y_abs >= 0) {
		coords.X = x_abs;
	}
	else {
		coords.X += x;
	}

	if (y_abs >= 0){
		coords.Y = y_abs;
	}
	else {
		coords.Y += y;
	}

	SetConsoleCursorPosition(CONSOLE_OUTPUT, coords);
	return coords;
}
