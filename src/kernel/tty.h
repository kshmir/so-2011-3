#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "scheduler.h"

#ifndef _TTY_H_
#define _TTY_H_

#define TTY_MAX_NUMBER			8
#define BUFFER_SIZE 			255
#define	TTY_LEFT 				-1
#define TTY_RIGHT 				1


// Creates TTY process
int tty_init(int tty_num);

// Moves TTY to a direction.
void switch_tty(int direction); 










void startKeyboard(int id);

char scanCodeToChar(char scanCode);

int canRead();

int setCanRead(int c);

/** Stores all the characters it receives on a buffer */
void pushC(char c);

/** Receives the scancodes which aren't from a key.
 * And changes the shift state, control, alt, etc.
 */
int controlKey(int scancode);

/** Gets whether caps is on or not */
int capsOn();
/** Tells wether a shift's on */
int isShifted();
/** Checks shifts and caps to check whether font should
 * be CAPITALIZED or not
 */
int isCapital();

/** Get's the last char on the KB buffer */
char getC();

int directionKey();

#endif