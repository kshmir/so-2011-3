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
#define PROCESS_STACK_SIZE		16384
#define PROCESS_MAX				64
#define PROCESS_WAIT_MAX		64
#define PROCESS_HISTORY_SIZE	100
#define PROCESS_SIGNALS			10

// File types
#define MON_FT_UNKNOWN		0		// Unknown File Type
#define MON_FT_REG_FILE		1		// Regular File
#define MON_FT_DIR			2		// Directory File
#define MON_FT_CHRDEV		3		// Character Device
#define MON_FT_BLKDEV		4		// Block Device
#define MON_FT_FIFO			5		// Buffer File
#define MON_FT_SOCK			6		// Socket File
#define MON_FT_SYMLINK		7		// Symbolic Link

// Extra param for FS
#define	O_CREAT					0x800		// Deletes the old file if exists.
#define	O_RD					0x400		// Reads from frome
#define	O_WR					0x200		// Writes to file.
#define	O_NEW					0x100		// Creates a new only if exists

#define	ERR_GENERAL			-1
#define	ERR_PERMS			-2
#define	ERR_NO_EXIST		-3
#define	ERR_EXISTS			-4
#define	ERR_REPEATED		-5
#define	ERR_INVALID_TYPE	-6
#define	ERR_INVALID_PATH	-7

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

/* pwd
 * Returns:
 * - String containing the current directory the tty is in.
 **/
char * pwd();

/* cd
 * Returns:
 * 1 if moved, 0 if not.
 **/
int cd(char * to);

/* mount
 * Returns:
 * List of EXT2 directory entries in the raw buffer.
 **/
int mount();


/* cd
 * Returns:
 * List of EXT2 directory entries in the raw buffer.
 **/
int mkdir(char * name);

int rm(char * name);


int getuid(char * username); // if null... returns current UID

int getgid(int uid);

int makeuser(char * username, char * password);

int setgid(char * username, int gid);

int udelete(char * username);

int uexists(char * username);

int ulogin(char * username, char * password);

int chown(char * filename, char * username);

int chmod(char * filename, int perms);

int fgetmod(char * filename);

int fgetown(char * filename);

int cp(char * from, char * to);

int mv(char * from, char * to);

int makelink(char * filename, char * target);

void fsstat(int * data);

void sleep(int msecs);

int logout();

#endif
