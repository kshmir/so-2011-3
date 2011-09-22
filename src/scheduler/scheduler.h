#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

////// Begin defines for PROCESS

// Process' states
#define PROCESS_RUNNING 		3
#define PROCESS_READY 			2
#define PROCESS_BLOCKED 		1
#define PROCESS_ZOMBIE 			0

// Process' attributes
#define	PROCESS_FD_SIZE			64
#define PROCESS_STACK_SIZE	4096
#define PROCESS_MAX 1024

////// End defines for PROCESS

////// Definition of process main.
typedef int (*main_pointer)(int argc, char **params);

////// Proper definition of Process
typedef struct Process {
	unsigned int 		pid;
	unsigned int 		gid;
	unsigned int 		priority;
	char 						stack[PROCESS_STACK_SIZE];
	int							state;
	int							is_tty;
	int 						esp;
	int							file_descriptors[PROCESS_FD_SIZE];
	int							calls;
} Process;

////// Stackframe, built on process creation.
typedef struct StackFrame {
	int 		EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX,  EIP, CS, EFLAGS;
	void	*	retaddr;
	int 		argc;
	char** 	argv;
} StackFrame;

////// Begin Scheduler functions

// Starts the processes n' stuff.
void scheduler_init();

Process * create_process(char * name, main_pointer _main, size_t stack_size, int priority);

// Begin Context Change functions.

void scheduler_save_esp (int esp);

void * scheduler_get_temp_esp (void);

void * scheduler_think (void); 

int scheduler_load_esp(Process * proc);

// End Context Change functions.

////// Begin Scheduler functions

#endif