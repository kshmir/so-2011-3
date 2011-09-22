#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "scheduler.h"

#ifdef _TTY_H_
#define _TTY_H_

#define	TTY_LEFT -1
#define TTY_RIGHT 1

// Creates TTY process
int tty_init();

// Moves TTY to a direction.
void switch_tty(int direction); 

#endif