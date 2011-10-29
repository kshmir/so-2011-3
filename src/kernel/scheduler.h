#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"

#include "../libs/queue.h"
#include "../monix/monix.h"

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

////// Definition of process main.
typedef int (*main_pointer)(int argc, char **params);

////// Proper definition of Process
typedef struct Process {
	unsigned int		pid;
	unsigned int		ppid;
	unsigned int		gid;
	unsigned int		priority;
	char *				name;
	char				stack[PROCESS_STACK_SIZE];
	int					state;
	int					is_tty;
	int					tty;
	int					esp;
	int					file_descriptors[PROCESS_FD_SIZE];
	int					open_fds;
	int					sleeptime;
	void *				signals[PROCESS_SIGNALS];
	Queue				* wait_queue;
} Process;

////// Stackframe, built on process creation.
typedef struct StackFrame {
	int 		EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX,  EIP, CS, EFLAGS;
	void	*	retaddr;
	int 		argc;
	char	** 	argv;
} StackFrame;

////// Begin Scheduler functions

// Starts the processes n' stuff.
void scheduler_init();

Process * getp();

int current_p_tty();

int process_getfreefd();

void process_cleanpid(int pid);

void process_setready(Process * p);

void make_atomic();

void release_atomic();

Process * create_process(char * name, main_pointer _main, int priority, unsigned int tty, 
	int is_tty, int stdin, int stdout, int stderr, int argc, void * params, int queue_block);

// Begin Context Change functions.

void scheduler_save_esp (int esp);

void * scheduler_get_temp_esp (void);

void scheduler_think (void); 

int scheduler_load_esp();

int sched_fork();

int sched_getpid();

void scheduler_tick();

Process * process_getbypid(int pid);

Process * process_getbypindex(int index);

main_pointer sched_ptr_from_string(char * string);

int sched_pdup2(int pid, int fd1, int fd2);

int sched_prun(int pid);

void * storage_index();

void yield();

void softyield();

// End Context Change functions.

////// Begin Scheduler functions

#endif