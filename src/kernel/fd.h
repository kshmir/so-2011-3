// The file descriptor is the core of our interaction between the possible filetypes in our system.
// This API provides a flexible solution for working with hdd files, fifos, and the keyboard/video.

#ifndef _FD_H_
#define _FD_H_

// Types of file descriptors
#define _FD_TTY       1
#define _FD_FIFO      2
#define _FD_FILE      3

#define _FD_MAX       1024

// Opens a file descriptor and makes an ID for it.
int fd_open_with_index (int fd, int type, void * data, int perms);

// Opens a file
int fd_open (int type, void * data, int params);

// Tells the type of a file.
int fd_type (int fd);

// Reads from a file descriptor
int fd_read (int fd, char * buffer, int block_size);

// Writes from a file descriptor
int fd_write(int fd, char * buffer, int block_size);

// Closes a file descriptor
int fd_close(int fd);
#endif