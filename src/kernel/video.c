/*
 * video.c
 *
 *  Created on: May 11, 2011
 *      Author: cristian
 */

#include "video.h"
#include "kernel.h"
#include "../../include/defs.h"
#include "../../include/kasm.h"
#include "../kernel/scheduler.h"
#include "../libs/stdlib.h"

int defaultStyle = 0x07;

VIDEO_MODE_INFO* current_video_mode;
// Counter of IRQ8 ticks since start.
int ticks = 0;
int cursor_ticks = 0;

// Counter of the video position
int videoPos = 0;

// Sets or not the cursor
int cursorEnabled = 1;
int hardCursorEnabled = 1;

// Direction to the video memory.
char *vidmem = (char *) 0xb8000;

int fix_flag = 0;

void video_write_c(char * data) {
	make_atomic();
	char a[] = { *data, defaultStyle };
	char c = *data;

	if (c == '\r') {
		backSpace();
	} else if (c == '\n') {
		newLine();
	} else if (c == 0x0f || c == '\t') {
		if (getCursorX() % 4 == 0) {
			int i = 0;
			for (i = 0; i < 4; ++i) {
				putChar(' ');
				*a = ' ';
				video_write(a, 2);
			}
		} else
			while (getCursorX() % 4 != 0) {
				putChar(' ');
				*a = ' ';
				video_write(a,2);
				incrementCursor();
			}
	} else if (c != 0) {
		putChar(c);
		if(current_video_mode->visible)	{
			video_write(a,2);
			incrementCursor();
		} else {
			setCursor(FALSE);
			incrementCursor();
			setCursor(TRUE);
		}
	}
	release_atomic();
}

void video_write(char * data, int count) {
	memcpy(vidmem + videoPos, data, count);
}

// TODO: Move me.
void setCursor(int b) {
	cursorEnabled = b;
	hardCursorEnabled = b;
}

// TODO: Move me.
void setVideoPos(int a) {
	videoPos = a;
	if (cursorEnabled) { 
		_setCursor(a / 2);
	}
}

void setVideoMode(VIDEO_MODE_INFO * m) {
	current_video_mode = m;
}

VIDEO_MODE_INFO* getVideoMode() {
	return current_video_mode;
}

// Starts default video.
void initVideo() {
	clear_screen();
	setCursorX(0);
	setCursorY(0);
}

void video_reload() { 
	int x, y;
	int old_x = getCursorX();
	int old_y = getCursorY();
	setCursor(FALSE);
	for(x = 0; x < current_video_mode->width; ++x)
	{
		for(y = 0; y < current_video_mode->height; ++y)
		{
			setCursorX(x);
			setCursorY(y);
			putC(current_video_mode->screen[x][y]);
		}
	}
	setCursor(TRUE);
	setCursorX(old_x);
	setCursorY(old_y);
}

// For future use, builds a given video mode.
VIDEO_MODE_INFO* buildVideoMode(int height, int width, int cursorX,
		int cursorY, int cursorEnabled, int textMode) {
	VIDEO_MODE_INFO* video = NULL;
	int i = 0;
	video = (VIDEO_MODE_INFO*) malloc(sizeof(VIDEO_MODE_INFO));
	video->height = height;
	video->width = width;
	video->curX = cursorX;
	video->curY = cursorY;
	video->cursorEnabled = cursorEnabled;
	video->textMode = textMode;
	video->visible = 0;
	video->screen = (char**) malloc(sizeof(char**) * width);
	for (i = 0; i < width; i++)	{
		video->screen[i] = (char*) malloc(sizeof(char*) * height);
	}
	video->style = (char**) malloc(sizeof(char**) * width);
	for (i = 0; i < width; i++)	{
		video->style[i] = (char*) malloc(sizeof(char*) * height);
	}
	return video;
}

// Puts a character to stdout
void putC(char c) {
	if(!in_kernel()) {
		write(STDOUT,&c,1);
	} else if(current_video_mode->visible)	{
		if(c != 1) {
			char a[] = { c, defaultStyle };
			video_write(a,2);
			incrementCursor();
		}
	}
}


int getCursorX() {
	return current_video_mode->curX;
}
int getCursorY() {
	return current_video_mode->curY;
}
void setCursorX(int x) {
	if (x >= 0 && x <= current_video_mode->width) {
		current_video_mode->curX = x;
		if(current_video_mode->visible)
		{
			setVideoPos(
					(current_video_mode->width * current_video_mode->curY
							+ current_video_mode->curX) * 2);
		}
	}
}
void setCursorY(int y) {
	if (y >= 0 && y <= current_video_mode->height) {
		current_video_mode->curY = y;
		if(current_video_mode->visible)
		{
			setVideoPos(
					(current_video_mode->width * current_video_mode->curY
						+ current_video_mode->curX) * 2);
		}
	}
}

void moveCursorToStart() {
	setCursorX(0);
	setCursorY(0);
}

void incrementCursor() {
	if (getCursorX() >= current_video_mode->width) {
		setCursorX(0);
		if (getCursorY() >= current_video_mode->height - 1)
		{
			newLine();
			setCursorY(current_video_mode->height - 1);
		}
		else
			setCursorY(getCursorY() + 1);
		return;
	}
	setCursorX(getCursorX() + 1);
}
void decrementCursor() {
	if (getCursorX() < 1) {
		if (getCursorY() < 1) {
			setCursorY(0);
			setCursorX(current_video_mode->width);
		} else {
			setCursorY(getCursorY() - 1);
			setCursorX(current_video_mode->width);
		}
		return;
	}
	setCursorX(getCursorX() - 1);
}

void clear_screen() {
	int i = 0;
	moveCursorToStart();
	setCursor(FALSE);
	while (i++ < (current_video_mode->width * (current_video_mode->height))) {
		putchar(' ');
	}
	setCursor(TRUE);
	moveCursorToStart();
}

// Clears the screen from the given cursor to the end of the page.
// And rolls the cursor back.
void clear_screen_topdown() {
	int i = 0;
	int x = getCursorX();
	int y = getCursorY();
	setCursor(FALSE);
	while (i++ < (current_video_mode->width * (current_video_mode->height
			- y)) - x) {
		putchar(' ');
	}
	setCursor(TRUE);
	setCursorX(x);
	setCursorY(y);
}
