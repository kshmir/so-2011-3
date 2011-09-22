#include "scheduler.h"
#include "../libs/string.h"

int _first_time          = 0;
int thinked              = 0;
int _processes_available = PROCESS_MAX;

unsigned int current_pid = 0;


static Process				* idle;
static Process				* kernel;
static Process				* p1;
static Process 				process_pool[PROCESS_MAX];

///////////// Inicio Funciones Scheduler

// TODO: Make memcpy for this.
void process_pool_start() {
	int i = 0;
	char *pool = (char*)	process_pool;
	for(; i < PROCESS_MAX * sizeof(Process); i++) {
		*pool = (char) 0;
	}
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
	} else 	if(strcmp(name, "kernel") == 0) {
		kernel = p;
	}
	else 	if(strcmp(name, "p1") == 0) {
		p1 = p;
	}
	
	return p;
}


void scheduler_save_esp (int esp)
{
	Process * temp;

	if (current_pid != 0)
	{
		temp      = process_getbypid(current_pid);
		temp->esp = esp;
	}
	return;
}

void * scheduler_get_temp_esp (void) {
	return (void*)idle->esp;
}

void* scheduler_think (void) {


	if(thinked == 0)
	{
		thinked     = 1;
		current_pid = kernel->pid;

		return kernel;
	} else {
		thinked     = 0;
		current_pid = p1->pid;

		return p1;

	}
}

int scheduler_load_esp(Process * proc)
{
	return proc->esp;
}

///////////// Fin Funciones Scheduler
