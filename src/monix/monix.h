/*****************************************************************************
 *
 *  MonkeyOS Kernel System Calls - "MONIX, like POSIX but smaller"
 *
 *****************************************************************************/
#ifndef _MONIX_H_
#define _MONIX_H_

#include "../../include/defs.h"

// Process' states
#define PROCESS_RUNNING 		3
#define PROCESS_READY 			2
#define PROCESS_BLOCKED 		1
#define PROCESS_ZOMBIE 			0

// Process' attributes
#define	PROCESS_FD_SIZE			64
#define PROCESS_STACK_SIZE		4096
#define PROCESS_MAX				64
#define PROCESS_WAIT_MAX		64
#define PROCESS_HISTORY_SIZE	100
#define PROCESS_SIGNALS			10

// Signals
#define SIGINT					2
#define	SIGKILL					9

/* write
 * Parameters:
 * - File Descriptor
 * - Buffer to write
 * - Length of the buffer
 **/
int write(unsigned int fd, const void* buffer, size_t count);

/* read
 * Parameters:
 * - File Descriptor
 * - Buffer to read
 * - Length of the buffer
 **/
int read(unsigned int fd, void* buffer, size_t count);

/* open
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int open(const char * fname, int perms);

/* close
 * Parameters:
 * - File Descriptor to close.
 **/
int close(int fd);

/* mkfifo
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the fifo.
 **/
int mkfifo(const char * fname, int perms);

/* fork
 * Returns:
 * - -1 if it's the new process, the child's PID if it's the parent.
 **/
int pcreate(void * name, int argc, char ** argv);

/* fork
 * Returns:
 * - -1 if it's the new process, the child's PID if it's the parent.
 **/
int prun(int pid);

/* fork
 * Returns:
 * - -1 if it's the new process, the child's PID if it's the parent.
 **/
int pdup2(int pid, int fd1, int fd2);

int getpid();

int waitpid(int pid);

/* open
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int openfifo(int fifo_id);

/* open
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int * pticks();

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
char * pname(int pid);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int pstatus(int pid);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int ppriority(int pid);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int pgid(int pid);


/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int pgetpid_at(int index);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int kill(int signal, int pid);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int psetp(int pid, int priority);

/* pname
 * Parameters:
 * - Filename
 * - Permissions
 * Returns:
 * - File Descriptor to the file, whichever type it has.
 **/
int psetsched(int schedmode, int priority);

#endif
