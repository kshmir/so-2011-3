#include "tty.h"
#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/mcglib.h"
#include "../libs/queue.h"
#include "scheduler.h"
#include "kernel.h"
#include "fd.h"


static int tty_index = 0;

extern int current_tty;

int process_input(const char * input, int tty_number) { 
	int piped = 0;
	char file_call_buffer[1024];
	char stdout_call_buffer[128];
	char stdin_call_buffer[128];
	
	int read_ptr = 0;
	int stdin = 0;
	int stdout = 0;
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
		
		int n = 0;
		char** strs = split_string(file_call_buffer, ' ', &n);
		
		
		int pid = pcreate(strs[0], n, strs);
		
		if(pid != -1)
		{
			if(stdin){
				pdup2(pid, stdin, STDIN);
			}
			
			if(piped)
			{
				char cad[20];
				itoa(pid, cad);
				stdout = mkfifo(cad);
				pdup2(pid,stdout,STDOUT);
				
				stdin = stdout;
				stdout = 0;
			}
			
			prun(pid);
			if(!string_ends_with(file_call_buffer, '&') && !piped) {
				waitpid(pid);
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
	init_context(tty_index);
	tty_index++;
	
	char cadena[50];
	
	while(1) {
		int val = 0;
		int tty_number = atoi(argv[1]);
		printf("user@tty%s:", argv[1]);

		char * input = (char *) getConsoleString(val);
		process_input(input, tty_number);
	}
	return 0;
}



static char * process_name = "tty";
// Creates TTY process
int tty_init(int tty_num) { 
	char ** _params = (char **)malloc(sizeof(char *) * 2);
	char * _num = (char *)malloc(sizeof(char) * 9);
	_params[0] = process_name;
	_params[1] = _num;
	itoa(tty_num, _num);
	fd_open_with_index(FD_TTY0 + tty_num, _FD_TTY, NULL, 0666);
	create_process("tty", tty_main, 0, tty_num, 1, FD_TTY0 + tty_num, FD_TTY0 + tty_num, FD_TTY0 + tty_num, 1, _params, 0);
}