#include "fd.h"
#include "video.h"

#define FILE_MAX 4096

typedef struct file {
	int    type;
	void * data;
	int    uid;
	int    perms;
	int    used;	
} file;


static unsigned int pipe_allocator       = 100;
static unsigned int pipe_allocator_index = 100;
static unsigned int pipe_allocator_size  = 0;
static unsigned int pipe_allocator_end   = 1024;
static unsigned int fd_allocator         = 1024;
static unsigned int fd_allocator_index   = 1024;
static unsigned int fd_allocator_size  = 0;
static unsigned int fd_allocator_end     = FILE_MAX;

static int files_allocd = 0;
static file files[FILE_MAX];

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


int fd_open (int type, void * data) {
	int fd = 0;

	if(type == _FD_PIPE) {
		pipe_allocator_size++;
		if(pipe_allocator_size == pipe_allocator_end - pipe_allocator - 1) {
			return -1;
		}
		fd = pipe_allocator_index++;
		if(pipe_allocator_index == pipe_allocator_end - 1) {
			pipe_allocator_index = pipe_allocator;
		}
	} else 	if(type == _FD_FILE) {
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

	return fd_open_with_index(fd, type, data);
}

int fd_open_with_index (int fd, int type, void * data) {
	if(files_allocd == 0) {
		files_alloc();
	}
	files[fd].used = 1;
	files[fd].type = type;
	files[fd].data = data;
	return fd;
}

int fd_type (int fd) {
	return files[fd].type;
}

int fd_read (int fd, char * buffer, int block_size) {
	switch(files[fd].type) {
		case _FD_TTY:
			// getc
		break;
		case _FD_PIPE:
			// pipe_read
		break;
		case _FD_FILE:
			// hdd_read
		break;
		default:
		return -1;
	}
}

int fd_write(int fd, char * buffer, int block_size) {
	switch(files[fd].type) {
		case _FD_TTY:
			video_write(buffer, block_size);
		break;
		case _FD_PIPE:
			fifo_write(fd, buffer, block_size);
		break;
		case _FD_FILE:
			// hdd_write
		break;
		default:
		return -1;
	}
}

int fd_close(int fd) {
	files[fd].used = 0;
}