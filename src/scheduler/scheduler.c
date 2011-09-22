#include "scheduler.h"
#include "../libs/string.h"
#include "../libs/queue.h"

int _first_time          = 0;
int thinked              = 0;
int _processes_available = PROCESS_MAX;

unsigned int current_pid = 0;


static Process				* idle;
static Process 				process_pool[PROCESS_MAX];
static Process				* current_process = NULL;
static Queue 					* ready_queue;
static Queue 					* blocked_queue;

///////////// Inicio Funciones Scheduler

// TODO: Make memcpy for this.
void scheduler_init() {
		printf("init");
	int i = 0;
	char *pool = (char*)	process_pool;
	for(; i < PROCESS_MAX * sizeof(Process); i++) {
		*pool = (char) 0;
	}

	ready_queue = queue_init(PROCESS_MAX);
	blocked_queue = queue_init(PROCESS_MAX);
}

unsigned int _pid_seed = 0;

unsigned int process_getnextpid() {
	return _pid_seed++;
}

Process * process_getbypid(int pid) {
	int i = 0;
	for(; i < PROCESS_MAX; ++i)
	{
		if(process_pool[i].state != PROCESS_ZOMBIE
		    && process_pool[i].pid == pid)
		{
			return &process_pool[i];
		}
	}
	return NULL;
}

Process * process_getfree() {
	if(_processes_available == 0) {
		return NULL; // Big problem.
	}
	int i = 0;
	for(; i < PROCESS_MAX; ++i) {
		if(process_pool[i].state == PROCESS_ZOMBIE) {
			_processes_available--;
			return process_pool + i;
		}
	}
	
	// Another big problem.
	return NULL;
}


void process_cleaner() {
	
}


int	stackf_build(void * stack, main_pointer _main) {
	
	void * bottom 	= (void *)((int)stack + PROCESS_STACK_SIZE -1);
	
	StackFrame * f	= (StackFrame *)(bottom - sizeof(StackFrame));
	
	f->EBP		=	0;
	f->EIP		=	(int)_main;
	f->CS			=	0x08;
	
	f->EFLAGS		=	0x202;
	f->retaddr	=	process_cleaner;
	f->argc			=	0;
	f->argv			=	NULL;
	
	return	(int)f;
}

Process * create_process(char * name, main_pointer _main, size_t stack_size, int priority) {
	Process * p = process_getfree();
	p->pid      = process_getnextpid();
	p->gid      = 0;
	p->priority = priority;
	p->esp      = stackf_build(p->stack, _main);
	p->state    = PROCESS_READY;

	if(strcmp(name, "idle") == 0) {
		idle = p;
	}
	
	queue_enqueue(ready_queue, p);
	
	return p;
}


void scheduler_save_esp (int esp)
{
	if (current_process != NULL) {
		current_process->esp = esp;
	}
}

void * scheduler_get_temp_esp (void) {
	return (void*)idle->esp;
}

void* scheduler_think (void) {
	if(current_process != NULL)
	{
		queue_enqueue(ready_queue, current_process);
	}
	
	return current_process = queue_dequeue(ready_queue);
}

int scheduler_load_esp(Process * proc)
{
	return proc->esp;
}

///////////// Fin Funciones Scheduler
