/*****************************************************************************
 *
 *  MonkeyOS Kernel System Calls - "MONIX, like POSIX but smaller"
 *
 *****************************************************************************/
#ifndef _MONIX_H_
#define _MONIX_H_

#include "defs.h"


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
int fork();

/* exec
 * Returns:
 * - Does not return to the process :)
 **/
int exec(const char * fname, const char * args);
 
/* dup2
 * Params:
 * - File Descriptor to change into fd2.
 * - File Descriptor to close and replace.
 **/
int dup2(int fd1, int fd2);


#endif
