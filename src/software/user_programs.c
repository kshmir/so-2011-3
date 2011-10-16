#include "user_programs.h"
#include "../monix/monix.h"
#include "../libs/pqueue.h"

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
		printf("HOLA :) %d\n",i);
	}

	return 0;
}

int writer_main (int argc, char ** argv)
{
	if(argc > 1)
	{
		int fifo = mkfifo("teta", 0666);
		int i = 1;
		for(; i < argc; ++i)
		{
			printf("param %d %s\n", i, argv[i]);
			write(fifo, argv[i], strlen(argv[i]));
		}

	} else { 
		printf("Not enough params\n");
	}

	return 0;
}

int reader_main (int argc, char ** argv) {
	int fifo = mkfifo("teta", 0666);

	
	char buff[1024];
	int len = read(fifo, buff, 1024);	
	int i = 0;
	printf("I READ RAW:");
	for(;i < len; i++)	{
		printf("%c", buff[i], i);
	}
	printf("\n");
	return 0;
}


int putc_main (int argc, char ** argv) {
	if(argc > 1) {
		printf("%s\n", argv[1]);
	}

	return 0;
}


int top_main  (int argc, char ** argv) {
	printf("TOP\n");	
	printf("PRESS q TO EXIT\n");
	char c;
	do {


		int allow_zombies = 0;

		if (argc > 1 && !strcmp(argv[1], "zombies")) {
			allow_zombies = 1;
		}

		int len = 0;
		int i = 0;
		int active_pids[PROCESS_MAX];
		int	active_pids_ticks[PROCESS_MAX];
		int active_pids_n = 0;

		for(; i < PROCESS_MAX; ++i)
		{
			active_pids[i] = -1;
			active_pids_ticks[i] = 0;
		}

		int tick_history[PROCESS_HISTORY_SIZE];
		int * _pticks = pticks();

		i = 0;

		for(; i < PROCESS_HISTORY_SIZE; i++) {
			if(_pticks[i] == -1) {
				break;
			}
			tick_history[i] = _pticks[i];
		}


		len = i;
		i = 0;
		int zombs = 0;
		for(; i < len; ++i)
		{
			int pid = tick_history[i];
			int _pid_index = -1, j = 0;
			for(; j < active_pids_n; j++)	{
				if(active_pids[j] == pid) {
					_pid_index = j;
					break;
				}
			}
			if(_pid_index == -1)	{
				_pid_index = active_pids_n;
				active_pids[_pid_index] = pid;
				active_pids_n++;
			}
			if (pstatus(pid) != PROCESS_ZOMBIE) {
				active_pids_ticks[_pid_index]++;
			} else {
				zombs++;
			}
		}

		if (!allow_zombies) {
			len -= zombs;
		}

		i = 0;
		int printed = 0;
		for(; i < PROCESS_MAX; ++i)	{
			int pid = pgetpid_at(i);

			if (pid != -1){
				int j = -1;
				for(; j < active_pids_n; j++)	{
					if(active_pids[j] == pid) {
						break;
					}
				}

				char * _pname = pname(pid);
				char * status = NULL;


				int stat = pstatus(pid);
				switch(stat){
					case PROCESS_READY:
					status = "READY";
					break;
					case PROCESS_BLOCKED:
					status = "BLOCKED";
					break;
					case PROCESS_ZOMBIE:
					status = "ZOMBIE ";
					break;
					case PROCESS_RUNNING:
					status = "RUNNING";
					break;
					default:
					status = "UNKNOWN";
				}

				int priority = ppriority(pid);
				int ticks = ( j == -1 ) ? 0 : active_pids_ticks[j];
				len = (!len) ? 1 : len;

				if (stat != PROCESS_ZOMBIE || (stat == PROCESS_ZOMBIE && allow_zombies) ) {
					if (printed % 25 == 23) {
						printf("PRESS A KEY TO CONTINUE LISTING...");
						getC();
					}
					printed++;
					
					printf("PID: %d \t NAME: %s \t CPU:%% %d \t STATUS: %s \t PRIORITY: %d\n",
						pid, _pname, 
						(100 * ticks) / len, status, priority);
				}
			}
		}
		printf("--------------------------------------------------------------------------------\n");
	}
	while((c = getC()) != 'q');
	
	return 0;
}

int getc_main (int argc, char ** argv) {	
	int c;
	while((c = getC()) != 1) {
		printf("%c", c + 1);
	}
	
	return 0;
}


int _kill (int argc, char ** argv)
{
	int c = getC();
	printf("%c\n", c);
	if (argc > 1) {
		int pid = atoi(argv[1]);
		if (pid >= 0) {
			kill(2,pid);
		}
	}
	return 0;
}

int _setp (int argc, char ** argv)
{
	if (argc > 2) {
		int priority = atoi(argv[2]);
		int pid = atoi(argv[1]);
		if (priority >= 0 && pid >= 0) {
			psetp(pid,priority);
		}
	}
	return 0;
}

int _setsched (int argc, char ** argv)
{
	if (argc > 1) {
		int priority = atoi(argv[1]);
		if (priority >= 0) {
			setsched(priority);
			if(priority == 1) {
				printf("Scheduler set to priority mode\n");
			}
			else if(priority == 0) {
				printf("Scheduler set to round robin mode\n");
			}
		}
	}
	return 0;
}

int _hang (int argc, char ** argv)
{
	while(1);
	return 0;
}