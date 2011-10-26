#include "scheduler.h"
#include "tty.h"
#include "fd.h"
#include "kernel.h"
#include "signals.h"
#include "../software/user_programs.c"
#include "../libs/string.h"
#include "../libs/queue.h"
#include "../monix/monix.h"

int _first_time          = 0;
int thinked              = 0;
int _processes_available = PROCESS_MAX;

unsigned int current_pid = 0;

static int	think_storage_index = 0;
static int	think_storage[PROCESS_HISTORY_SIZE];

static Process				* idle;
static Process 				process_pool[PROCESS_MAX];
static Process				* current_process = NULL;
static Queue 				* ready_queue;
static Queue 				* yield_queue;
static PQueue 				* priority_queue;

///////////// Inicio Funciones Scheduler

#define	MODE_ROUNDR		0
#define	MODE_PRIORITY	1

static int interrupts = 0;

int Sti() {
		_Sti();
}

int Cli() {
		_Cli();
}


int sched_mode = MODE_ROUNDR;

/** Swithches the scheduler mode to round robin or priority*/
void sched_set_mode(int m) {
	if (m == 0 || m == 1) {
		sched_mode = m;
		
		Process * p;		
		if (m == 1) {
			while(!queue_isempty(ready_queue)) {
				p = queue_dequeue(ready_queue);
				pqueue_enqueue(priority_queue,p,p->priority);
			}
		} else if (m == 0){
			while(!pqueue_isempty(priority_queue)) {
				p = pqueue_dequeue(priority_queue);
				queue_enqueue(ready_queue,p);
			}
		}
		
	}
}
/** Returns true if there are not processes in the queue*/
int sched_isempty() {
	switch(sched_mode) {
		case MODE_PRIORITY:
			return pqueue_isempty(priority_queue);
			break;
		case MODE_ROUNDR:
		default:
			return queue_isempty(ready_queue);		
			break;
	}
}
/** Adds a process to the ready queue*/
void sched_enqueue(Process * p) {
	switch(sched_mode) {
		case MODE_PRIORITY:
//			printf("Enqueuing in pq %d %d %d\n", priority_queue, p, p->priority);
			pqueue_enqueue(priority_queue,p,p->priority);
			break;
		case MODE_ROUNDR:
		default:
			queue_enqueue(ready_queue,p);		
			break;
	}
}
/** Removes a process from the reasy queue*/
void * sched_dequeue() {
	switch(sched_mode) {
		case MODE_PRIORITY:
//					printf("Dequeuing in pq\n");
			return pqueue_dequeue(priority_queue);
			break;
		case MODE_ROUNDR:
		default:
			return queue_dequeue(ready_queue);		
			break;
	}
}
/** Tells the scheduler to add the process to the ready queue*/
void process_setready(Process * p) { 
	
	sched_enqueue(p);
}

int out = 0;
/** Initializes the process pool, ready queue, yield queue and priority queue*/
void scheduler_init() {
	int i = 0;
	char *pool = (char*)	process_pool;
	for(; i < PROCESS_MAX * sizeof(Process); i++) {
		*pool = (char) 0;
	}
	
	i = 0;
	for(; i < PROCESS_MAX; ++i)	{
		process_pool[i].state = -1;
	}
	ready_queue   = queue_init(PROCESS_MAX);
	yield_queue   = queue_init(PROCESS_MAX);
	priority_queue = pqueue_init(10, PROCESS_MAX);
	
	i = 0;
	for(; i < PROCESS_HISTORY_SIZE; ++i) {
		think_storage[i] = -1;
	}
}

unsigned int _pid_seed = 0;

unsigned int process_getnextpid() {
	return _pid_seed++;
}

/** Returns the process at the position index of the process pool*/
Process * process_getbypindex(int index) {
	return &process_pool[index];
}
/** Returns a process by knowing it's process id*/
Process * process_getbypid(int pid) {
	int i = 0;
	for(; i < PROCESS_MAX; ++i)	{
		if(process_pool[i].pid == pid)	{
			return &process_pool[i];
		}
	}
	return NULL;
}

/** Returns a zombie process or a wrong process from the process pool.*/
Process * process_getfree() {
	if(_processes_available == 0) {
		return NULL; // Big problem.
	}
	int i = _pid_seed;
	if(i >= PROCESS_MAX - 1)	{
		i = 0;
	}
	for(; i < PROCESS_MAX; ++i) {
		if(process_pool[i].state == PROCESS_ZOMBIE || process_pool[i].state == -1) {
			_processes_available--;
			return process_pool + i;
		}
		if(i >= PROCESS_MAX - 1)	{
			i = 0;
		}
	}

	return NULL;
}
/** Returns a file descriptor which is not used by the current process*/
int process_getfreefd() {
	if(current_process->open_fds == PROCESS_FD_SIZE) {
		return -1; // Big problem.
	}
	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i) {
		if(current_process->file_descriptors[i] == -1) {
			return i;
		}
		if(i == PROCESS_FD_SIZE - 1)	{
			i = 0;
		}
	}

	return -1;
}
/** Returns the terminal of the current process*/
int current_p_tty() {
	return current_process->tty;
}
/** Returns current process.*/
Process * getp() {
	return current_process;
}

int ran = 0;
/** Removes all the process in the wait queue of pid, closes the file descriptors 
of the pid and turns it to zombie*/
void process_cleanpid(int pid) {

	Process * _curr = process_getbypid(pid);

	while(!queue_isempty(_curr->wait_queue)) {
		Process * p = queue_dequeue(_curr->wait_queue);
		p->state = PROCESS_READY;
		sched_enqueue(p);
	}

	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i)	{
		fd_close(_curr->file_descriptors[i]);
	}

	_curr->state = PROCESS_ZOMBIE;
	_processes_available++;
	softyield();
}
/** Cleans current process (same as above)*/
void process_cleaner() {

	while(!queue_isempty(current_process->wait_queue)) {
		Process * p = queue_dequeue(current_process->wait_queue);
		p->state = PROCESS_READY;
		sched_enqueue(p);
	}

	int i = 0;
	for(; i < PROCESS_FD_SIZE; ++i)	{
		close(i);
	}

	current_process->state = PROCESS_ZOMBIE;
	_processes_available++;
	softyield();
}


int yielded = 0;
int yield_save_cntx = 0;
int soft_yielded = 0;

/*
	Switches cards, allows the process to wait for something to come soon. It does NOT save CPU.
	The process must be saved somewhere else!!!
*/
void softyield() {
	soft_yielded = 1;
	_yield();
}

/*
	Allows the process to wait for the next tick, it saves CPU.
*/
void yield() {
	queue_enqueue(yield_queue, current_process);
	yield_save_cntx = 1;
	yielded++;
	_yield();
}
/** Copies the fd1 to the f2*/
int sched_pdup2(int pid, int fd1, int fd2) {
	Process * p = process_getbypid(pid);
	fd_close(p->file_descriptors[fd2]);
	p->file_descriptors[fd2] = current_process->file_descriptors[fd1];
	return 0;
}
/** Takes the process pid from the pool and inserts it into the queue*/
int sched_prun(int pid) {
	Process * p = process_getbypid(pid);
	sched_enqueue( p);
	return pid;
}
/** Inserts the process in the waitting queue of the current 
process (or himself if it's the tty the current process)*/
int sched_waitpid(int pid) {
	Process * p = process_getbypid(pid);
	if(p != NULL)
	{
		if (current_process->is_tty) {
			set_owner_pid(p->pid);
		}
		queue_enqueue(p->wait_queue, current_process);
		current_process->state = PROCESS_BLOCKED;
		return pid;
	} else {
		return -1;
	}
}
/** Initializes a process*/
int sched_pcreate(char * name, int argc, void * params) {

	void * ptr = sched_ptr_from_string(name);
	if(ptr == NULL)	{
		return -1;
	}
	
	char * _name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
	int i = 0, len = strlen(name);
	for(; i < len; ++i)
	{
		_name[i] = name[i];
	}
	
	

	Process * p = create_process(_name, ptr, current_process->priority, current_process->tty, 0, 
		FD_TTY0 + current_process->tty, FD_TTY0 + current_process->tty, FD_TTY0 + current_process->tty, 
		argc, params, 1);
	return p->pid;
}

// Function names
char* _function_names[] = { "help", "test", "clear", "ssh", "hola", "reader", "writer", 
	"kill", "getc", "putc", "top", "hang", "setp", "setsched", "dcheck", 
	"dread", "dwrite", "dfill", "ls", "cd", "pwd", "mkdir", "rm", "touch", "cat", "fwrite",
	"logout", "makeuser", "setgid", "udelete", "chown", "chmod", "getown", "getmod", "fbulk", NULL };

// Functions
int ((*_functions[])(int, char**)) = { _printHelp, _test, _clear, _ssh, _hola_main, 
	reader_main, writer_main, _kill, getc_main, putc_main, top_main, _hang, 
	_setp, _setsched, _dcheck, _dread, _dwrite, _dfill, _ls, _cd, _pwd, _mkdir, _rm, _touch,
	_cat, _fwrite, _logout, _makeuser, _setgid, _udelete, _chown, _chmod, _getown, _getmod, _fbulk, NULL };

main_pointer sched_ptr_from_string(char * string) {
	int index;
	for (index = 0; _function_names[index] != NULL; ++index) {
		int n = 0;
		if (!strcmp(string, _function_names[index]) && strlen(string) >= strlen(_function_names[index])) {
			return _functions[index];
		}
	}
	return NULL;
}



int sched_getpid() {
	return current_process->pid;
}
/** Builds the stack frame of a process*/
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
/** Creates a process setting it's attributes and enqueues it in the ready list*/
Process * create_process(char * name, main_pointer _main, int priority, unsigned int tty, 
int is_tty, int stdin, int stdout, int stderr, int argc, void * params, int queue_block) {
	Process * p            = process_getfree();
	p->name                = name;
	p->pid                 = process_getnextpid();
	p->gid                 = 0;
	p->ppid                = (current_process != NULL) ? current_process->pid : 0;
	p->priority            = priority;
	p->esp                 = stackf_build(p->stack, _main, argc, params);
	p->state               = PROCESS_READY;
	p->file_descriptors[0] = fd_open_with_index(stdin,0,0,0);
	p->file_descriptors[1] = fd_open_with_index(stdout,0,0,0);
	p->file_descriptors[2] = fd_open_with_index(stderr,0,0,0);
	p->is_tty              = is_tty;
	p->wait_queue          = queue_init(PROCESS_WAIT_MAX);
	
	sg_set_defaults(p);
	

	int i;
	for(i = 3; i < PROCESS_FD_SIZE; ++i) {
		p->file_descriptors[i] = -1;
	}

	p->tty = tty;


	if(strcmp(name, "idle") == 0) {
		idle = p;
	} else if(!queue_block)	{
		sched_enqueue( p);
	}


	return p;
}

/** Saves process stack pointes*/
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

long c = 0;

int atomic = 0;

void make_atomic() {
	atomic++;
}

void release_atomic() {
	atomic--;
}

/** Here the scheduler decides which will be the next process to excecute*/
void scheduler_think (void) {

	if(atomic || in_kernel()) {
		return;
	}


	if(yielded == 0) {
		while(!queue_isempty(yield_queue)) {
			sched_enqueue(queue_dequeue(yield_queue));
		}
	} else { 
		yielded--;
	}

	if (current_process != NULL                       // Kinda old condition, stays for good
		&& current_process->state == PROCESS_RUNNING  // This way zombies 'pass out'
		&& current_process != idle)
	{
		current_process->state = PROCESS_READY;
		sched_enqueue(current_process);
		current_process = NULL;
	}

	
	if (!sched_isempty()) {
		current_process = sched_dequeue();
		
		// Process wakeup
		if (current_process->state == PROCESS_BLOCKED) {
			current_process->state = PROCESS_READY;
			soft_yielded = 1;
		}
	
		if (current_process->state == PROCESS_ZOMBIE) {
			softyield();
		}
	}
	else {
		current_process = idle;
	}
	
	if (soft_yielded) {
		soft_yielded = 0;
	} else {
		if(current_process != idle)
		{
			think_storage[think_storage_index++] = current_process->pid;
			if (think_storage_index == PROCESS_HISTORY_SIZE) {
				think_storage_index = 0;
			}
		}
	}

	switch_tty(current_process->tty);
	if (current_process != idle) {
		current_process->state = PROCESS_RUNNING;

	}
}

void * storage_index() {
	return think_storage;
}

int scheduler_load_esp()
{
	return current_process->esp;
}

///////////// Fin Funciones Scheduler
