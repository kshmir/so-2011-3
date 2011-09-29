#include "scheduler.h"
#include "tty.h"
#include "../libs/string.h"
#include "../libs/queue.h"

int _first_time          = 0;
int thinked              = 0;
int _processes_available = PROCESS_MAX;

unsigned int current_pid = 0;


static Process				* idle;
static Process 				process_pool[PROCESS_MAX];
static Process				* current_process = NULL;
static Queue 				* ready_queue;
static Queue 				* yield_queue;
static Queue 				* blocked_queue;

///////////// Inicio Funciones Scheduler


void scheduler_init() {
	int i = 0;
	char *pool = (char*)	process_pool;
	for(; i < PROCESS_MAX * sizeof(Process); i++) {
		*pool = (char) 0;
	}

	ready_queue   = queue_init(PROCESS_MAX);
	blocked_queue = queue_init(PROCESS_MAX);
	yield_queue   = queue_init(PROCESS_MAX);
}

unsigned int _pid_seed = 0;

unsigned int process_getnextpid() {
	return _pid_seed++;
}

void waitProcess(Process * p) {
	while(p->state != PROCESS_ZOMBIE) { 
		yield();
	}
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

int current_p_tty() {
	return current_process->tty;
}

Process * getp() {
	return current_process;
}


void process_cleaner() {
	current_process->state = PROCESS_ZOMBIE;
	_yield();
}


int yielded = 0;
int yield_save_cntx = 0;

void softyield() {
	queue_enqueue(ready_queue, current_process);
	_yield();
}

void yield() {
	queue_enqueue(yield_queue, current_process);
	yield_save_cntx = 1;
	yielded++;
	_yield();
}

int	stackf_build(void * stack, main_pointer _main, int argc, void * argv) {
	
	void * bottom 	= (void *)((int)stack + PROCESS_STACK_SIZE -1);
	
	StackFrame * f	= (StackFrame *)(bottom - sizeof(StackFrame));
	
	f->EBP     = 0;
	f->EIP     = (int)_main;
	f->CS      = 0x08;
	
	f->EFLAGS  = 0x202;
	f->retaddr = process_cleaner;
	f->argc    = argc;
	f->argv    = argv;

	
	return	(int)f;
}

Process * create_process(char * name, main_pointer _main, int priority, unsigned int tty, 
						int is_tty, int stdin, int stdout, int stderr, int argc, void * params) {
	Process * p            = process_getfree();
	p->pid                 = process_getnextpid();
	p->gid                 = 0;
	p->ppid                = (current_process != NULL) ? current_process->pid : 0;
	p->priority            = priority;
	p->esp                 = stackf_build(p->stack, _main, argc, params);
	p->state               = PROCESS_READY;
	p->file_descriptors[0] = stdin;
	p->file_descriptors[1] = stdout;
	p->file_descriptors[2] = stderr;
	p->is_tty 			   = is_tty;
	
	p->tty 		= tty;
	

	if(strcmp(name, "idle") == 0) {
		idle = p;
	} else {
		queue_enqueue(ready_queue, p);
	}
	
	return p;
}


void scheduler_save_esp (int esp)
{

	if (current_process != NULL) {
		current_process->esp = esp;
		if(yield_save_cntx)
		{
			current_process = NULL;
			yield_save_cntx = 0;
		}
	}
}

void * scheduler_get_temp_esp (void) {
	if(idle != NULL)
	{
		return (void*)idle->esp;
	} 
	else
	{
		return NULL;
	}

}

int c = 0;

void scheduler_think (void) {
	
	if(yielded == 0) {
		while(!queue_isempty(yield_queue)) {
			queue_enqueue(ready_queue, queue_dequeue(yield_queue));
		}
	} else { 
		yielded--;
	}
	
	if (current_process != NULL 
		&& current_process->state != PROCESS_ZOMBIE)
	{
		queue_enqueue(ready_queue, current_process);
	}
	
	if(!queue_isempty(ready_queue))
	{
		current_process = queue_dequeue(ready_queue);
	} 
	else
	{
		current_process = idle;
	}
	
	if(c % 20 == 0) {
		//printf("TTY: %d\n", current_process->tty);
//		printf("%d\n", queue_count(ready_queue));
	}

	c++;

	switch_tty(current_process->tty);
}

int scheduler_load_esp()
{
	if(current_process == NULL)
	{
		return 0;
	}
	return current_process->esp;
}

///////////// Fin Funciones Scheduler
