#include "signals.h"
#include "scheduler.h"

/* Kills the process with the pid recieved*/
void sigkill_h(int pid){
	process_cleanpid(pid);
}
/* Kills the process with the pid recieved*/
void sigint_h(int pid){
	process_cleanpid(pid);
}
/* Clears every previous functions and asigns only sigkill and sigint signals*/
void sg_set_defaults(Process * p) {
	int i = 0;
	for(; i < PROCESS_SIGNALS; ++i)	{
		p->signals[i] = NULL;
	}
	p->signals[SIGKILL] = sigkill_h;
	p->signals[SIGINT]  = sigint_h;
}
/* Sets a handler to a signal except whrn it's a SIGKILL which can't be overriden*/
void sg_set(int sig_n, void * handler) {
	if (sig_n != SIGKILL)	{
		Process * current = getp();
		current->signals[sig_n] = handler;
	}
}
/* Excecutes the handler of the signal sig_n from the process pid*/
void sg_handle(int sig_n, int pid) {
	Process * current = process_getbypid(pid);
	if (current->signals != NULL)	{
		((void (*)(int))current->signals[sig_n])(pid);
	}
}