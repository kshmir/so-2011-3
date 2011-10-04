/*****************************************************************************
 *
 *  MonkeyOS Kernel System Calls - "MONIX, like POSIX but smaller"
 *
 *****************************************************************************/
#ifndef _MONIX_H_
#define _MONIX_H_

#include "../../include/defs.h"


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
#endif
