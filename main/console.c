#include "shell.h"

/* -----WINDOWS----
* Returns the coordinate of the top visible line of the console
* Return: Integer value of top coordinate
*/
int getConsoleTop(void){
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	return screen_info.srWindow.Top;
}

/* -----WINDOWS----
* Returns the coordinate of the bottom visible line of the console
* Return: Integer value of bottom coordinate
*/
int getConsoleBottom(void){
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	return screen_info.srWindow.Bottom;
}

/* -----WINDOWS----
* Clears a console line
* Params: Number of characters to clear, x coord, y coord, attributes to set line to
*/
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

void clearScreen(){
	int height = getConsoleHeight();
	int width = getConsoleWidth();
	for (int i = 0; i < height; i++){
		clearLine(width, 1, i, NORMAL_ATTRIBUTES);
	}
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
* Prints a wchar string to a location in the CONSOLE_OUTPUT with a specific set of attributes
* Params: Wchar string to print, handle of CONSOLE_OUTPUT, x coord, y coord, attributes
*/
void advPrint(wchar_t *content, HANDLE CONSOLE_OUTPUT, int x, int y, WORD attributes){
	COORD coords;
	COORD oldCoords;
	CONSOLE_SCREEN_BUFFER_INFO screen_info;
	GetConsoleScreenBufferInfo(CONSOLE_OUTPUT, &screen_info);
	oldCoords.X = screen_info.dwCursorPosition.X;
	oldCoords.Y = screen_info.dwCursorPosition.Y;

	// If x and y are -1 then we should just print at the current location
	if (x == -1 || y == -1){
		coords.X = oldCoords.X;
		coords.Y = oldCoords.Y;
	}
	else if (y == 0){
		coords.Y = getConsoleTop();
		coords.X = x;
	}
	else {
		coords.X = x;
		coords.Y = y;
	}

	// If no attributes are supplied then it's a darkish gray
	if (attributes == NULL){
		attributes = POSSIBLE_ATTRIBUTES[NORMAL_ATTRIBUTES];
	}

	SetConsoleCursorPosition(CONSOLE_OUTPUT, coords);
	SetConsoleTextAttribute(CONSOLE_OUTPUT, attributes);
	wprintf(L"%s", content);
	SetConsoleCursorPosition(CONSOLE_OUTPUT, oldCoords);
}

void setTransparency(int t){
	HWND ConsoleWindow = GetConsoleWindow();
	SetWindowLong(ConsoleWindow, GWL_EXSTYLE, GetWindowLong(ConsoleWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(ConsoleWindow, 0, t, LWA_ALPHA);
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
