/*
 * video.h
 *
 *  Created on: May 11, 2011
 *      Author: cristian
 */

#ifndef VIDEO_H_
#define VIDEO_H_


#include "../../include/defs.h"


#define VIDEO 0xb8000

void initVideo();

void setCursor(int b);

void setVideoMode(VIDEO_MODE_INFO* vid);

VIDEO_MODE_INFO* getVideoMode();

VIDEO_MODE_INFO* buildVideoMode(int height, int width, int cursorX, int cursorY, int cursorEnabled, int textMode);

void incrementCursor();

void video_write(char * data, int count);

void decrementCursor();

int getCursorX();

int getCursorY();

void video_reload();

void setCursorX(int x);
void setCursorY(int y);

void moveCursorToStart();

void putC(char c);

void clear_screen();

#endif /* VIDEO_H_ */
