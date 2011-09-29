#ifndef _FD_H_
#define _FD_H_

#define _FD_TTY       1
#define _FD_PIPE      2
#define _FD_FILE      3

#define _FD_MAX       1024

int fd_open_with_index (int fd, int type, void * data);

int fd_open (int type, void * data);

int fd_type (int fd);

int fd_read (int fd, char * buffer, int block_size);

int fd_write(int fd, char * buffer, int block_size);

int fd_close(int fd);

#endif