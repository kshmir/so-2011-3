#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/queue.h"
#include "scheduler.h"

static int 	tty_index = 0;

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
		int val = 0;
		char * input = (char *) getConsoleString(val);
		printf("%s %d\n", input, _GetESP());
		_Halt();
	}
	return 0;
}

// Creates TTY process
int tty_init(int tty_num) { 
	create_process("tty", tty_main, 0, 0, tty_num);
}