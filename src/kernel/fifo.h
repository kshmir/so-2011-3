#ifndef _FIFO_H_
#define _FIFO_H_

int fifo_exists(char * file_name);

int fifo_make(int inode);

int fifo_open(char * file_name);

int fifo_write(int fd, char * msg, int len);

int fifo_read(int fd, char * buffer, int block_size);

int fifo_close(int fd);

#endif