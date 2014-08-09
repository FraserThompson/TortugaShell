#include "shell.h"
#include "console.h"

/* -----WINDOWS----
* Displays the strings in a settings menu.
* Params: Array of strings to display, integer values of attributes in POSSIBLE_ATTRIBUTES, number of strings, width of box, current cursor, number of cells to offset the location by in X and Ys
* Return: COORD of the cursor after shifting
*/
COORD draw_options(wchar_t **options, int *attributes, int num_options, int width, COORD current_cursor, int offsetX, int offsetY){
	int i = 0;
	int option_len = 0;
	int option_center = 0;
	int consoleWidth = getConsoleWidth(CONSOLE_OUTPUT);
	WORD select_attributes = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;

	// Drawing options
	for (i = 0; i < num_options; i++){
		option_len = wcslen(options[i]);
		option_center = (consoleWidth / 2 - option_len / 2) + offsetX;
		current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
		advPrint(options[i], CONSOLE_OUTPUT, option_center, current_cursor.Y, POSSIBLE_ATTRIBUTES[attributes[i]]);
		advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, select_attributes);
		advPrint(L" ", CONSOLE_OUTPUT, current_cursor.X + width - 2, current_cursor.Y, select_attributes);
	}

	return current_cursor;
}

/* -----WINDOWS----
* Clears a box in a specified area.
* Params: COORD of current cursor, height of box, width of box, offset x and y
* Return: COORD of the cursor after shifting
*/
COORD clear_a_box(COORD current_cursor, int height, int width,  int offsetX, int offsetY){
	int i = 0;
	int consoleWidth = getConsoleWidth(CONSOLE_OUTPUT);
	int consoleHeight = getConsoleHeight(CONSOLE_OUTPUT);
	int screenCenterX = (consoleWidth / 2 - width / 2) + offsetX;
	int screenCenterY = (consoleHeight / 2 - height / 2) + offsetY;
	int test;
	current_cursor = moveCursor(0, 0, screenCenterX, screenCenterY, CONSOLE_OUTPUT);

	for (i = 0; i < height + 1; i++){
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, NULL, width + 1, current_cursor, &test);
		FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width + 1, current_cursor, &test);

		current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
	}

	current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
	return current_cursor;
}

/* -----WINDOWS----
* Draws a box in the center of the screen
* Params: COORD of current cursor, attributes for border, attributes for fill, title, footer, height of box, width, speed to draw it in ms, offsetx and y in cells
* Return: COORD of the cursor after shifting
*/
COORD draw_a_box(COORD current_cursor, WORD border_attributes, WORD fill_attributes, wchar_t *title, wchar_t *footer, int height, int width, int speed, int offsetX, int offsetY){
	int i = 0;
	int consoleWidth = getConsoleWidth(CONSOLE_OUTPUT);
	int consoleHeight = getConsoleHeight(CONSOLE_OUTPUT);
	int title_len = wcslen(title);
	int footer_len = wcslen(footer);
	int center = width / 2 - title_len / 2;
	int screenCenterX = (consoleWidth / 2 - width / 2) + offsetX;
	int screenCenterY = (consoleHeight / 2 - height / 2) + offsetY;
	int test = 0;
	current_cursor = moveCursor(0, 0, screenCenterX, screenCenterY, CONSOLE_OUTPUT);

	// Drawing perimeter
	advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	for (i = 0; i < height; i++){
		Sleep(speed);
		current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
		FillConsoleOutputAttribute(CONSOLE_OUTPUT, fill_attributes, width, current_cursor, &test);
		FillConsoleOutputCharacter(CONSOLE_OUTPUT, L' ', width, current_cursor, &test);
		advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	for (i = 0; i < width; i++){
		Sleep(speed);
		current_cursor = moveCursor(1, 0, -1, -1, CONSOLE_OUTPUT);
		advPrint(L"=", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	for (i = 0; i < height; i++){
		Sleep(speed);
		current_cursor = moveCursor(0, -1, -1, -1, CONSOLE_OUTPUT);
		advPrint(L"|", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}
	for (i = 1; i < width; i++){
		Sleep(speed);
		current_cursor = moveCursor(-1, 0, -1, -1, CONSOLE_OUTPUT);
		advPrint(L"=", CONSOLE_OUTPUT, current_cursor.X, current_cursor.Y, border_attributes);
	}

	//header
	center = (consoleWidth / 2 - title_len / 2) + offsetX;
	advPrint(title, CONSOLE_OUTPUT, center, current_cursor.Y, border_attributes);

	//footer
	center = (consoleWidth / 2 - footer_len / 2) + offsetX;
	advPrint(footer, CONSOLE_OUTPUT, center, current_cursor.Y + height, border_attributes);

	current_cursor = moveCursor(0, 1, -1, -1, CONSOLE_OUTPUT);
	return current_cursor;
}

/* -----WINDOWS----
* Draws a a settings box by calling draw_a_box and draw_options
* Params: Attributes for border, attributes for filling, title, footer, string array of options, integer array of attributes, number of options, height, width, speed in ms, offsetx and offset y
* Return: COORD of the cursor after shifting
*/
COORD draw_settings(WORD border_attributes, WORD fill_attributes, wchar_t *title, wchar_t *footer, wchar_t **options, int *attributes, int num_options, int height, int width, int speed, int offsetX, int offsetY){
	COORD current_cursor = getCursor(CONSOLE_OUTPUT);
	COORD options_cursor;
	current_cursor = draw_a_box(current_cursor, border_attributes, fill_attributes, title, footer, height, width, speed, offsetX, offsetY);
	options_cursor = current_cursor;
	current_cursor = draw_options(options, attributes, num_options, width, current_cursor, offsetX, offsetY);
	current_cursor = moveCursor(0, -(num_options - 1), -1, -1, CONSOLE_OUTPUT);
	return options_cursor;
}
