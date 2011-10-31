#ifndef _FIFO_H_
#define _FIFO_H_

// Tells if a fifo exists or not 
int fifo_exists(int key);

// Sets up a fifo in memory with it's key
int fifo_make(int key);

// Writes inside a fifo, might block.
int fifo_write(int fd, char * msg, int len);

// Reads a fifo, it might block.
int fifo_read(int fd, char * buffer, int block_size);


// Closes a fifo, formality :p
int fifo_close(int fd);
#endif