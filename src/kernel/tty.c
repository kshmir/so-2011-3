#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/mcglib.h"
#include "../libs/queue.h"
#include "scheduler.h"


static int 	tty_index = 0;

extern int current_tty;

// Helps teachers to understand a bit our mess, well, no
int _printHelp(int size, char** args) {
	printf("MonkeyOS 1 - MurcielagOS kernel v0.1 (i686-pc-murcielago)\n");
	printf("These shell commands are defined internally.  Type `help' to see this list.\n");
}

// Test the breakable code
int _test(int size, char** args) {

	printf("Welcome to the test programme\n\n");
	printf("We are gonna test the following code:\n");

	char name[200];
	char surname[200];
	int age = 0;
	double height = 2.0;

	printf("char name[200];\nchar surname[200];\nint age = 0;\n");
	printf("double height = 0.0;\nprintf(\"please enter your name:\");\n");
	printf("scanf(\"%%s\",name);\n");
	printf("printf(\"please enter you age:\");\n");
	printf("scanf(\"%%d\",&age);\n");
	printf("printf(\"please enter you height:\");\n");
	printf("scanf(\"%%d\",&height);\n");
	printf(
			"printf(\"my name is:%%s i am %%d years old and %%d feet tall\\n\",name,age,height);\n");
	printf(
			"printf(\"please enter your name surname \\nfor example Bruce Wayne:\");\n");
	printf("scanf(\"%%s %%s\",name,surname);\n");
	printf(
			"printf(\"my name is %%s and my surname is %%s\\n\",name,surname);\n");

	printf("\nPress ENTER to begin the test");
	getchar();
	printf("\n\nplease enter your name:");
	scanf("%s", name);
	printf("please enter you age:");
	scanf("%d", &age);
	printf("please enter you height:");
	scanf("%f", &height);
	printf("my name is:%s i am %d years old and %f feet tall\n", name, age, height);
	printf("please enter your name surname \nfor example Bruce Wayne:");
	scanf("%s %s", name, surname);
	printf("my name is %s and my surname is %s\n", name, surname);
}

// Just to have more functions in the autocomplete
int _ssh(int size, char** args) {
	printf("Attempting to connect...\n");
	printf("Oooops we forgot our internals don't have any TCP/IP backend...\n");
	printf("You'll have to wait for MurcielagOS 2.0 or maybe 11.0 to see this working\n");
	printf("Take a seat and wait!!!\n");
	printf("...\n");
	printf("...\n");
	printf("...\n");
	printf("Our autocomplete looked just empty so we made this :)\n");
}

// Clears the screen
int _clear(int size, char** args) {
	clear_screen();
}


int _hola_main (int argc, char ** argv)
{
	int i = 0;
	for(; i < 100; ++i) {
		printf("HOLA :)\n");
	}

	return 0;
}

int writer_main (int argc, char ** argv)
{
	if(argc > 1)
	{
		int fifo = fifo_open("teta");
		int i = 1;
		for(; i < argc; ++i)
		{
			printf("param %d %s\n", i, argv[i]);
			fifo_write(fifo, argv[i], strlen(argv[i]));
		}

	} else { 
		printf("Not enough params\n");
	}

	return 0;
}

int reader_main (int argc, char ** argv) {
	int fifo = fifo_open("teta");

	
	char buff[1024];
	int len = fifo_read(fifo, buff, 1024);	
	int i = 0;
	printf("I READ RAW:");
	for(;i < len; i++)
	{
		printf("%c", buff[i], i);
	}
	printf("\n");
	
	return 0;
}


// Function names
char* _function_names[] = { "help", "test", "clear", "ssh", "hola", "reader", "writer", NULL };

// Functions
int ((*_functions[])(int, char**)) = { _printHelp, _test, _clear, _ssh, _hola_main, reader_main, writer_main, NULL };

int process_input(const char * input) { 
	int piped = 0;
	char file_call_buffer[1024];
	char stdout_call_buffer[128];
	char stdin_call_buffer[128];
	
	int read_ptr = 0;
	
	do {
		int i = 0;
		int write_i = 0;
		for(; i < 128; ++i) {
			stdout_call_buffer[i] = 0;
			stdin_call_buffer[i]  = 0;
		}

		for(i = 0; i < 1024; ++i)	{
			file_call_buffer[i] = 0;
		}
		i = read_ptr;
		
		char * file_call;
		int stdin = 0;
		int stdout = 0;
		
		int valid_input = 0;
		char * current_buffer = file_call_buffer;
		for(; input[i] && input[i] != '|'; ++i) {
			if(strlen(current_buffer) > 0 ) {
				if(input[i] == '>') {
					current_buffer = stdout_call_buffer;
					valid_input = 0;
					write_i = 0;
					continue;
				} else if (input[i] == '<') { 
					current_buffer = stdin_call_buffer;
					valid_input = 0;
					write_i = 0;
					continue;
				}
			} else {
				if (input[i] == '>' || input[i] == '<') {
					return 0;
				}
			}
			valid_input = input[i] != ' ';
			if(!(write_i == 0 && !valid_input)) {
				current_buffer[write_i++] = input[i];
			}
		}
		if(i == 0 && input[i] == '|') {
			return 0;
		}
		piped = (input[i] == '|'); 
		i++;
		
		if(piped)
		{
			read_ptr = i;
		}
		int index;
		for (index = 0; _function_names[index] != NULL; ++index) {
			int n = 0;
			if (!strcmp(file_call_buffer, _function_names[index]) && strlen(file_call_buffer) >= strlen(_function_names[index])) {
				int n = 0;
				char** strs = split_string(file_call_buffer, ' ', &n);
				if(!string_ends_with(file_call_buffer, '&'))
				{
					waitProcess(create_process(file_call_buffer, _functions[index], 0, current_tty, 0, 0, 0 ,0, n, strs));
				} else	{
					create_process(file_call_buffer, _functions[index], 0, current_tty, 0, 0, 0 ,0, n, strs);
				}
				 
				break;
			}
		}
		
//		printf("file %s stdout %s stdin %s\n",file_call_buffer, stdout_call_buffer,stdin_call_buffer);
	} while(piped);
	return 1;
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
		process_input(input);
	}
	return 0;
}

// Creates TTY process
int tty_init(int tty_num) { 
	create_process("tty", tty_main, 0, tty_num, 1, 0, 0, 0, 0, NULL);
}