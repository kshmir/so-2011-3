#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/queue.h"
#include "video.h"
#include "scheduler.h"
#include "kernel.h"

#define MAX 0xff
/* Pongo 2 coordenadas. 1: tecla normal. 2: Shift. */
/* Confio en que las entradas no completadas se completan con 0 = NPRTBL */

#define NPRTBL 0

unsigned char keyboard[][2] = { { NPRTBL, NPRTBL },//000
		{ NPRTBL, NPRTBL },//001 ESCAPE
		{ '1', '!' }, //002
		{ '2', '\"' }, //003
		{ '3', '#' }, //004
		{ '4', '$' }, //005
		{ '5', '%' }, //006
		{ '6', '&' }, //007
		{ '7', '/' }, //008
		{ '8', '(' }, //009
		{ '9', ')' }, //010
		{ '0', '=' }, //011
		{ '\'', '?' }, //012
		{ '\n', '\n' }, //013
		{ '\b', '\b' }, //014 BACKSPACE
		{ '\t', '\t' }, //015 TAB
		{ 'q', 'Q' }, //016
		{ 'w', 'W' }, //017
		{ 'e', 'E' }, //018
		{ 'r', 'R' }, //019
		{ 't', 'T' }, //020
		{ 'y', 'Y' }, //021
		{ 'u', 'U' }, //022
		{ 'i', 'I' }, //023
		{ 'o', 'O' }, //024
		{ 'p', 'P' }, //025
		{ '\'', '\"' }, //026
		{ '+', '*' }, //027
		{ '\n', '\n' }, //028
		{ NPRTBL, NPRTBL },//029 CTRL IZQ
		{ 'a', 'A' }, //030
		{ 's', 'S' }, //031
		{ 'd', 'D' }, //032
		{ 'f', 'F' }, //033
		{ 'g', 'G' }, //034
		{ 'h', 'H' }, //035
		{ 'j', 'J' }, //036
		{ 'k', 'K' }, //037
		{ 'l', 'L' }, //038
		{ '.', ';' }, //039
		{ '{', '[' }, //040
		{ '}', '|' }, //041
		{ NPRTBL, NPRTBL },//042 SHIFT IZQ
		{ '<', '>' }, //043
		{ 'z', 'Z' }, //044
		{ 'x', 'X' }, //045
		{ 'c', 'C' }, //046
		{ 'v', 'V' }, //047
		{ 'b', 'B' }, //048
		{ 'n', 'N' }, //049
		{ 'm', 'M' }, //050
		{ ',', ';' }, //051
		{ '.', ':' }, //052
		{ '?', '?' }, //053
		{ NPRTBL, NPRTBL },//054 SHIFT DER
		{ '*', '*' }, //055 KEY *
		{ NPRTBL, NPRTBL },//056 ALT IZQ
		{ ' ', ' ' }, //057 SPACE
		{ NPRTBL, NPRTBL },//058 CAPSLOCK
		{ NPRTBL, NPRTBL },//059 F1
		{ NPRTBL, NPRTBL },//060 F2
		{ NPRTBL, NPRTBL },//061 F3
		{ NPRTBL, NPRTBL },//062 F4
		{ NPRTBL, NPRTBL },//063 F5
		{ NPRTBL, NPRTBL },//064 F6
		{ NPRTBL, NPRTBL },//065 F7
		{ NPRTBL, NPRTBL },//066 F8
		{ NPRTBL, NPRTBL },//067 F9
		{ NPRTBL, NPRTBL },//068 F10
		{ NPRTBL, NPRTBL },//069 NUM LOCK
		{ NPRTBL, NPRTBL },//070 SCROLL LOCK
		{ '7', '7' }, //071 KEY 7
		{ '8', '8' }, //072 KEY 8
		{ '9', '9' }, //073 KEY 9
		{ '-', '-' }, //074 KEY -
		{ '4', '4' }, //075 KEY 4
		{ '5', '5' }, //076 KEY 5
		{ '6', '6' }, //077 KEY 6
		{ '+', '+' }, //078 KEY +
		{ '1', '1' }, //079 KEY 1
		{ '2', '2' }, //080 KEY 2
		{ '3', '3' }, //081 KEY 3
		{ '0', '0' }, //082 KEY 0
		{ '.', '.' }, //083 KEY .
		{ NPRTBL, NPRTBL },//084 SYS REQ (AT)
		{ '+', '*' }, //085
		{ '+', '*' }, //086
		{ NPRTBL, NPRTBL },//087 F11
		{ NPRTBL, NPRTBL },//088 F12
		{ '+', '*' }, //089
		{ '+', '*' } //090
		//Para asegurarme podria llenar con NPRTBL lo que queda
};

#define		TTY_KEYBOARD_BUFFER	256

typedef struct TTY_Context { 
	// Keyboard Context
	char	direction;
	char	numLock;
	char	capsLock;
	char	lShift;
	char	rShift;
	char	lCtrl;
	char	rCtrl;
	char	lAlt;
	char	rAlt;
	char	del;
	char	_escPressed;
	int		lastlastkey;
	int		lastkey;
	int		charBufferPointer;
	int		arrowBufferPointer;
	char	arrowBuffer[BUFFER_SIZE];
	char	charBuffer[BUFFER_SIZE];
	
	// Video Context
	VIDEO_MODE_INFO *	video_context;
} TTY_Context;

static TTY_Context	tty_contexts[TTY_MAX_NUMBER];
int					current_tty = 0;

void switch_tty(int number) {
	TTY_Context * cont = &tty_contexts[number]; 	
	setVideoMode(tty_contexts[number].video_context);
}

int aux = 0;
void init_context(int id) {
	aux = current_tty;
	TTY_Context * cont = &tty_contexts[id]; 
	startKeyboard(id);
	cont->video_context = buildVideoMode(25, 80, 1, 10, 10, 1);
	cont->video_context->visible = (id == 0);
	
	current_tty = id;

	setVideoMode(tty_contexts[current_tty].video_context);
	initVideo();
	current_tty = aux;
}

static TTY_Context * cnt() { 
	return &tty_contexts[current_tty];
}

int escPressed() {
	return cnt()->_escPressed;
}

void startKeyboard(int id)
{
	int i = 0;
	for (i = 0; i < BUFFER_SIZE; ++i) {
		tty_contexts[id].arrowBuffer[i] = 0;
		tty_contexts[id].charBuffer[i] = 0;
	}
	tty_contexts[id].direction          = 0;
	tty_contexts[id].numLock            = 0;
	tty_contexts[id].capsLock           = 0;
	tty_contexts[id].lShift             = 0;
	tty_contexts[id].rShift             = 0;
	tty_contexts[id].lCtrl              = 0;
	tty_contexts[id].rCtrl              = 0;
	tty_contexts[id].lAlt               = 0;
	tty_contexts[id].rAlt               = 0;
	tty_contexts[id].del                = 0;
	tty_contexts[id]._escPressed        = 0;
	tty_contexts[id].lastlastkey        = 0;
	tty_contexts[id].lastkey            = 0;
	tty_contexts[id].charBufferPointer  = -1;
	tty_contexts[id].arrowBufferPointer = -1;
}

// Buffer for arrows
void pushArr(char c) {
	if (cnt()->arrowBufferPointer >= BUFFER_SIZE - 1) {
		cnt()->arrowBufferPointer = BUFFER_SIZE - 2;
	}
	cnt()->arrowBuffer[++cnt()->arrowBufferPointer] = c;
}

void pushC(char c) {
	if (cnt()->charBufferPointer >= BUFFER_SIZE - 1){
		cnt()->charBufferPointer = BUFFER_SIZE - 2;
	}
	cnt()->charBuffer[++cnt()->charBufferPointer] = c;
}

char scanCodeToChar(char scanCode) {
	if ((cnt()->lCtrl || cnt()->rCtrl)) {
		int in = FALSE;
		tty_contexts[current_tty].video_context->visible = 0;
		switch(keyboard[scanCode][0]) {
			case '1':
				current_tty = 0;
				in = TRUE;
				break;
			case '2':
				current_tty = 1;
				in = TRUE;
				break;
			case '3':
				current_tty = 2;
				in = TRUE;
				break;
			case '4':
				current_tty = 3;
				in = TRUE;
				break;
			case '5':
				current_tty = 4;
				in = TRUE;
				break;
			case '6':
				current_tty = 5;
				in = TRUE;
				break;
			case '7':
				current_tty = 6;
				in = TRUE;
				break;
			case '8':
				current_tty = 7;
				in = TRUE;
				break;
		}
		tty_contexts[current_tty].video_context->visible = 1;
		if(in)
		{
			setVideoMode(tty_contexts[current_tty].video_context);
			video_reload();
			return 0;
		}

	}
	
	if (scanCode >= 0x02 && scanCode <= 0x0d)
		return keyboard[scanCode][isShifted()];
	return keyboard[scanCode][isCapital()];
}

int controlKey(int scancode) {
	
	if (scancode == 42) //SHIFT IZQ
		cnt()->lShift = 1;
	else if (scancode == 54) //054 SHIFT DER
		cnt()->rShift = 1;
	else {
		// Numpad
		if (cnt()->lastkey != -32)
			switch (scancode) {
			case 71:
				pushC('7');
				break;
			case 72:
				pushC('8');
				break;
			case 73:
				pushC('9');
				break;
			case 75:
				pushC('4');
				break;
			case 76:
				pushC('5');
				break;
			case 77:
				pushC('6');
				break;
			case 79:
				pushC('1');
				break;
			case 80:
				pushC('2');
				break;
			case 81:
				pushC('3');
				break;
			case 82:
				pushC('0');
				break;
			}
		else {
			// Arrows
			switch (scancode) {
			case 72:
				pushArr(8);
				break;
			case 75:
				pushArr(4);
				break;
			case 77:
				pushArr(6);
				break;
			case 80:
				pushArr(2);
				break;
			}

		}

		if (scancode == 0xFFFFFFAA)
			cnt()->lShift = 0;
		else if (scancode == 0xFFFFFFB6)
			cnt()->rShift = 0;
		else if (scancode == 0x1c)
			pushC('\n');
		else if (scancode == 0x38) {
			if (cnt()->lastkey != -32)
				cnt()->lAlt = 1;
			else
				cnt()->rAlt = 1;
		} else if (scancode == 0xFFFFFFB8) {
			if (cnt()->lastkey != -32) 
				cnt()->lAlt = 0;
			else
				cnt()->rAlt = 0;
		} else if (scancode == 0x1D) {
			if (cnt()->lastkey != -32)
				cnt()->lCtrl = 1;
			else
				cnt()->rCtrl = 1;
		} else if (scancode == 0xFFFFFF9D) {
			if (cnt()->lastkey != -32)
				cnt()->lCtrl = 0;
			else
				cnt()->rCtrl = 0;
		} else if (scancode == 0x01)
			cnt()->_escPressed = 1;
		else if (scancode == 0xFFFFFF81) //release esc
		{
			cnt()->_escPressed = 0;
		} else if (scancode == 83 && cnt()->lastkey == -32)
			cnt()->del = 1;
		else if (scancode == -45 && cnt()->lastkey == -32)
			cnt()->del = 0;
		else if (scancode == 0x45)
			cnt()->numLock = cnt()->numLock ? 0 : 1;
		else if (scancode == 0x0E)
			pushC('\r');
		else if (scancode == 0x3A)
			cnt()->capsLock = cnt()->capsLock ? 0 : 1;
		else {
			cnt()->lastlastkey = cnt()->lastkey;
			cnt()->lastkey = scancode;
			if (scancode == 0x39) { //space
				pushC(' ');
				return 1;
			} else if (scancode == 0x0f) { // tab
				pushC(scancode);
				return 1;
			}
		}
	}
	if ((cnt()->lAlt || cnt()->rAlt) && (cnt()->lCtrl || cnt()->rCtrl) && cnt()->del)
		_restart(); // Handles direct restart

	cnt()->lastlastkey = cnt()->lastkey;
	cnt()->lastkey = scancode;
	return 0;
}

char getA() {
	return 0; 
	
}

// char getC() {	
// 	while (cnt()->charBufferPointer < 0 
// 		   || current_tty != current_p_tty()) {
// 		yield();
// 	}
// 
// 	char ret = cnt()->charBuffer[cnt()->charBufferPointer--];
// 	return ret;
// }

int tty_read(char * buf, int len) {
	if(cnt()->charBufferPointer < 0 || current_tty != current_p_tty()) {
		return SYSR_BLOCK;
	}
	char ret = cnt()->charBuffer[cnt()->charBufferPointer--];
	* buf = ret;
	return 1;
}


char getC() {	
	char ret[1];
	read(STDIN, ret, 1);
	return *ret;
}

int capsOn() {
	return cnt()->capsLock;
}

int isCapital() {
	return ((cnt()->lShift || cnt()->rShift) && !cnt()->capsLock) 
			|| cnt()->capsLock && !(cnt()->lShift || cnt()->rShift);
}

int isShifted() {
	return cnt()->lShift || cnt()->rShift;
}