#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/list.h"

#include "scheduler.h"
#include "semaphore.h"
#include "fifo.h"

#define FIFO_DATA_SIZE 4

typedef struct fifo {
	int inode;
	char data[FIFO_DATA_SIZE];
	int	wr_i;
	int rd_i;
	int write_locked;
	int writes;
} fifo;

static list fifos = NULL;

static fifo * fifo_find(int inode) {
	foreach(fifo *, f, fifos) { 
		if(inode == f->inode)	{
			return f;
		}
	}
	return NULL;
}

int fifo_open(char * file_name) {
	if(fifos == NULL)	{
		fifos = list_init();
	}
	
	int n = 31; 
	
	// TODO: Use a filesystem function.
	int i = 0;
	int len = strlen(file_name);
	for(i = 0; i < len; ++i) {
		n += file_name[i] * file_name[i] * file_name[i];
	}
	
	fifo * f = NULL;
	if((f = fifo_find(n)) == NULL)
	{
		f               = (fifo *) malloc(sizeof(fifo));
		f->inode        = n;
		f->wr_i         = 0;
		f->rd_i         = 0;
		f->writes       = 0;
		f->write_locked = 0;
		i = 0;
		for(; i < FIFO_DATA_SIZE; ++i)	{
			f->data[i] = 0;
		}
		list_add(fifos, f);
	} 
	return (int)f;
}

int fifo_write(int fd, char * msg, int len){ 
	fifo * f = (fifo *) fd;
	size_t i = 0;
	for(; i < len; i++, f->wr_i++)
	{
		f->write_locked = 0;		
		while(f->writes == FIFO_DATA_SIZE) {
			f->write_locked = 1;
			softyield();
		}

		if(f->wr_i == FIFO_DATA_SIZE) {
			f->wr_i = 0;
		}
		f->data[f->wr_i] = msg[i];
		f->writes++;
	}	
}

int fifo_read(int fd, char * buffer, int read_size){	
	fifo * f = (fifo *) fd;
	while(f->writes == 0) {
		softyield();
	}
	size_t i = 0;
	for(; i < read_size && (f->writes > 0 || f->write_locked); i++, f->rd_i++)
	{
		while(f->writes == 0) {
			softyield();
		}
		
		if(f->rd_i == FIFO_DATA_SIZE) {
			f->rd_i = 0;
		}
		buffer[i] = f->data[f->rd_i];
		f->writes--;
	}
	return 1;
}

int fifo_close(int fd){
	// TODO;
	return 1;
}