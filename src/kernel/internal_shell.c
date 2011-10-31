#include "internal_shell.h"
#include "../kernel/scheduler.h"

// Makes a new line
void newLine() {
	_Cli();
	setCursorX(0);
	if (!(getCursorY() >= getVideoMode()->height - 1)) {
		setCursorY(getCursorY() + 1);
	} else {
		int i, j;
		int last_len = 0, current_len;

		for (j = 0; j < getVideoMode()->height; j++) {
			for (i = 0; i < getVideoMode()->width; i++) {
				getVideoMode()->screen[i][j]
					= getVideoMode()->screen[i][j + 1];
			}
		}
		for (i = 0; i < getVideoMode()->width; i++) {
			getVideoMode()->screen[i][getVideoMode()->height] = ' ';
		}
		reDrawLines();

		setCursorY(getCursorY() - 1);
		setCursorX(0);
	}
	_Sti();
}

/**	Copies everything from the screen buffer (Not the Video buffer) and re draw it*/
void reDrawLines() {
	_Cli();
	int i, j;
	setCursor(FALSE);
	setCursorY(0);
	for (j = 0; j < getVideoMode()->height; j++) {
		setCursorX(0);
		for (i = 0; i < getVideoMode()->width; i++) {
			putC(getVideoMode()->screen[i][j]);
		}
		setCursorY(getCursorY() + 1);
	}
	setCursor(TRUE);
	setCursorX(0);
	_Sti();
}

// Puts a space
void putSpace() {
	putC(getC());
	getVideoMode()->screen[getCursorX() + 1][getCursorY()] = ' ';
}

// Inserts a tab
void putTab() {
	putchar(0x0f);
}

// Makes a backspace
void backSpace() {
	if (getCursorX() % 4 == 0
			&& getVideoMode()->screen[getCursorX() - 1][getCursorY()]
					== 0x0f) {
		removeTab();
	} else {
		removeLastC();
	}

}

// Removes a tab
void removeTab() {
	int oneStep = 1;
	int x = getCursorX();
	while (x % 4 > 0 || oneStep) {
		if (getVideoMode()->screen[x - 1][getCursorY()] == 0x0f
				|| getVideoMode()->screen[x - 1][getCursorY()] == 0) {
			removeLastC();
		}
		oneStep = 0;
		x--;

	}
}

// Puts a char
void putChar(char c) {
	make_atomic();
	int x = getCursorX();
	int y = getCursorY();
	if (x <= getVideoMode()->width) {
		if (c != '\r')	{
			getVideoMode()->screen[x][y] = c;
		}
		x += (c != '\r') ? 1 : -1;
	} else {
		newLine();
		getVideoMode()->screen[x][y] = c;
	}
	release_atomic();
}

// Escape trigger, deprecated
void onEscape() {
	clear_screen();
}

// Backspace trigger
void removeLastC() {
	decrementCursor();
	putChar(' ');
	putC(' ');
	decrementCursor();
}

