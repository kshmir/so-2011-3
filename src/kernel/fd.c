#include "fd.h"
#include "video.h"
#include "kernel.h"
#include "tty.h"

#define FILE_MAX 4096

typedef struct file {
	int	   type;
	void * data;
	unsigned long    offset;
	int    fifo_inode;
	int    gid;
	int	   uid;
	int	   perms;
	int	   used;	
} file;

static unsigned int FIFO_allocator		 = 100;
static unsigned int FIFO_allocator_index = 100;
static unsigned int FIFO_allocator_size	 = 0;
static unsigned int FIFO_allocator_end	 = 1024;
static unsigned int fd_allocator		 = 1024;
static unsigned int fd_allocator_index	 = 1024;
static unsigned int fd_allocator_size  = 0;
static unsigned int fd_allocator_end	 = FILE_MAX;

static int files_allocd = 0;
static file files[FILE_MAX];
/* Initializes all file descriptors*/
static void files_alloc() {
	int i = 0;
	for(; i < FILE_MAX; ++i)
	{
		files[i].type  = 0;
		files[i].data  = (void *)0;
		files[i].uid   = 0;
		files[i].perms = 0600;
		files[i].used  = 0;
	}
	files_allocd = 1;
}
/* Used to find a FIFO fd having a key.*/
int fd_find(int type, int key) { 
	int i = 0;
	for(; i < FILE_MAX; ++i)
	{
		if(files[i].type == type && key == (int)files[i].data) {
			return i;
		}
	}
	return -1;
}

/* Opens a file descriptor whcich could be a FIFO or just a file. On error returns -1*/
int fd_open (int type, void * data, int perms) {
	int fd = 0;

	if(type == _FD_FIFO) {
		int key = fifo_exists(data);
		if(key) {
			return fd_find(type, key);
		}
		
		FIFO_allocator_size++;
		if(FIFO_allocator_size == FIFO_allocator_end - FIFO_allocator - 1) {
			return -1;
		}
		fd = FIFO_allocator_index++;
		if(FIFO_allocator_index == FIFO_allocator_end - 1) {
			FIFO_allocator_index = FIFO_allocator;
		}
	} else	if(type == _FD_FILE) {
		fd_allocator_size++;
		if(fd_allocator_size == fd_allocator_end - fd_allocator - 1) {
			return -1;
		}
		fd = fd_allocator_index++;
		if(fd_allocator_index == fd_allocator_end - 1) {
			fd_allocator_index = fd_allocator;
		}
	} else {
		return -1;
	}

	return fd_open_with_index(fd, type, data, perms);
}

/* Opens a file descriptor and sets it's type, data and permissions*/
int fd_open_with_index (int fd, int type, void * data, int perms) {
	if(files_allocd == 0) {
		files_alloc();
	}
	if(!files[fd].used)		{
		files[fd].type = type;
		files[fd].data = data;
		files[fd].perms = perms;
		switch(files[fd].type) {
			case _FD_TTY:
			//
			break;
			case _FD_FIFO:
				files[fd].fifo_inode = (int) fs_open_fifo(data, current_ttyc()->pwd, perms);                    // Fifos are stored with their own id...
				if(files[fd].fifo_inode > 0)	{
					files[fd].data = (void *) fifo_make(files[fd].fifo_inode);
				}
			break;
			case _FD_FILE:
				files[fd].data = (void *)fs_open_reg_file(data, current_ttyc()->pwd, perms);             // Inodes are stoded with their inode number.

				if(files[fd].data > 0 && fs_is_fifo((int)files[fd].data))
				{
					files[fd].fifo_inode = (int) files[fd].data;
					files[fd].data = (void *) fifo_make(files[fd].fifo_inode);
					files[fd].type = _FD_FIFO;
				}
			break;
			default:
			return -1;
		}
	}
	
	// Errors...
	if((int)files[fd].fifo_inode < 0 && type == _FD_FIFO)	{
		return (int)files[fd].fifo_inode;
	}
	if((int)files[fd].data < 0 && type == _FD_FILE)	{
		return (int)files[fd].data;
	}
	files[fd].used++;
	return fd;
}
/* Return the type of a fd (FILE, FIFO or TTY)*/
int fd_type (int fd) {
	return files[fd].type;
}
/* Reads the content of the file descriptor and stores it in the buffer.*/
int fd_read (int fd, char * buffer, int block_size) {
	switch(files[fd].type) {
		case _FD_TTY:
			return tty_read(buffer, block_size);
		break;
		case _FD_FIFO:
			return fifo_read((int)files[fd].data, buffer, block_size);
		break;
		case _FD_FILE:
			return fs_read_file((int)files[fd].data, buffer, block_size, &files[fd].offset);
		break;
		default:
		return -1;
	}
}
/* Writes the content of the buffer in the file descriptor.*/
int fd_write(int fd, char * buffer, int block_size) {
	switch(files[fd].type) {
		case _FD_TTY:
		if(block_size == 1 && *buffer != EOF)	{
			video_write_c(buffer);
		} else if (*buffer != EOF){
			video_write(buffer, block_size);
		}
		break;
		case _FD_FIFO:
			return fifo_write((int)files[fd].data, buffer, block_size);
		break;
		case _FD_FILE:
			return fs_write_file((int)files[fd].data, buffer, block_size);
		break;
		default:
		return -1;
	}
	return 1;
}
/* Closes the file descriptor and writes EOF to it.*/
int fd_close(int fd) {
	if (fd != -1 && files[fd].used)	{
		if(files[fd].used == 0)	{
			fifo_close(files[fd].data);
		}
		files[fd].used--;
	} else if (fd != -1){
		if(files[fd].used == 0)	{
			fifo_close(files[fd].data);
		}
	}
	// TODO: Make the remaining clears
}