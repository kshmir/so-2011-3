#include "signals.h"
#include "scheduler.h"


void sigkill_h(int pid){
	process_cleanpid(pid);
}

void sigint_h(int pid){
	process_cleanpid(pid);
}

void sg_set_defaults(Process * p) {
	int i = 0;
	for(; i < PROCESS_SIGNALS; ++i)	{
		p->signals[i] = NULL;
	}
	p->signals[SIGKILL] = sigkill_h;
	p->signals[SIGINT]  = sigint_h;
}

void sg_set(int sig_n, void * handler) {
	if (sig_n != SIGKILL)	{
		Process * current = getp();
		current->signals[sig_n] = handler;
	}
}

void sg_handle(int sig_n, int pid) {
	Process * current = process_getbypid(pid);
	if (current->signals != NULL)	{
		((void (*)(int))current->signals[sig_n])(pid);
	}
}