#include "scheduler.h"

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

void sg_set_defaults(Process * p);

void sg_set(int sig_n, void * handler);

void sg_handle(int sig_n, int pid);

#endif