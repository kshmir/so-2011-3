#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/queue.h"
#include "scheduler.h"

#define		TTY_MAX_NUMBER			8
#define		TTY_KEYBOARD_BUFFER	256

typedef struct TTY_Context { 
	Queue *	char_buffer;
} TTY_Context;

static int 					tty_index;
static TTY_Context	tty_contexts[TTY_MAX_NUMBER];
static int					current_tty;

void init_context(int id) {
	TTY_Context * cont = &tty_contexts[id]; 
	
}

int tty_main (int argc, char ** argv)
{
	if(tty_index >= TTY_MAX_NUMBER) {
		return 0;
	}
	
	char cadena[50];
	int 	tty_id = tty_index++;
	init_context(tty_id);
	printf("user@tty:");
	while(1) {

		_Halt();
	}
	return 0;
}

// Creates TTY process
int tty_init() { 
	create_process("tty", tty_main, 0, 0);
}



// Moves TTY to a direction.
void switch_tty(int direction) { 
	
}