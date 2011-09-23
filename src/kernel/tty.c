#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/queue.h"
#include "scheduler.h"

static int 	tty_index = 0;

int hola_main (int argc, char ** argv)
{
	int i = 0;
	for(; i < 100; ++i)
	{
		printf("HOLA :)\n");
	}

	return 0;
}

int tty_main (int argc, char ** argv)
{
	if(tty_index >= TTY_MAX_NUMBER) {
		return 0;
	}
	
	char cadena[50];
	int 	tty_id = tty_index++;
	init_context(tty_id);

	while(1) {
		int val = 0;
		printf("user@tty:");
		char * input = (char *) getConsoleString(val);
		if(strcmp(input, "hola") == 0 && strlen(input) == strlen("hola")) {
			waitProcess(create_process("hola", hola_main, 0, 0, 1, 0, 0, 0)); 
			yield();
		}
		if(strcmp(input, "hola&") == 0 && strlen(input) == strlen("hola&")) {
			printf("[%d]\n", create_process("hola", hola_main, 0, 0, 1, 0, 0, 0)->pid); 
		}
		

	}
	return 0;
}

// Creates TTY process
int tty_init(int tty_num) { 
	create_process("tty", tty_main, 0, tty_num, 1, 0, 0, 0);
}