#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/list.h"

#include "scheduler.h"
#include "semaphore.h"
#include "fifo.h"
#include "kernel.h"

#define FIFO_DATA_SIZE 4

typedef struct fifo {
	int inode;
	char data[FIFO_DATA_SIZE];
	int	wr_i;
	int rd_i;
	int buf_rd_i;
	int buf_wr_i;
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


int fifo_exists(char * file_name) {
	int n = 31; 
	
	// TODO: Use a filesystem function.
	int i = 0;
	int len = strlen(file_name);
	for(i = 0; i < len; ++i) {
		n += file_name[i] * file_name[i] * i;
	}	
	return (int)fifo_find(n);
}



int fifo_make(char * file_name) {
	if(fifos == NULL)	{
		fifos = list_init();
	}
	
	int n = 31; 
	
	// TODO: Use a filesystem function.
	int i = 0;
	int len = strlen(file_name);
	for(i = 0; i < len; ++i) {
		n += file_name[i] * file_name[i] * i;
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
		f->buf_wr_i     = 0;
		f->buf_rd_i     = 0;
		i = 0;
		for(; i < FIFO_DATA_SIZE; ++i)	{
			f->data[i] = 0;
		}
		list_add(fifos, f);
	} 
	return (int)f;
}

int	fifo_open(char * file_name) {
	int n = 31; 
	
	// TODO: Use a filesystem function.
	int i = 0;
	int len = strlen(file_name);
	for(i = 0; i < len; ++i) {
		n += file_name[i] * file_name[i] * i;
	}
	
	fifo * f = NULL;
	if((f = fifo_find(n)) == NULL) {
		return -1;
	} else {
		return (int)f;
	}
	
}

int fifo_write(int fd, char * msg, int len){ 
	fifo * f = (fifo *) fd;

	for(; f->buf_wr_i < len; f->buf_wr_i++, f->wr_i++)	{		
		if(f->writes == FIFO_DATA_SIZE) {
			f->write_locked = 1;
			printf("I should lock\n");
			return SYSR_BLOCK;
		}
		f->write_locked = 0;
		if(f->wr_i == FIFO_DATA_SIZE) {
			f->wr_i = 0;
		}
		f->data[f->wr_i] = msg[f->buf_wr_i];
		f->writes++;
	}	
	
	printf("I end :D\n");
	f->buf_wr_i     = 0;
}

int fifo_read(int fd, char * buffer, int read_size){	
	fifo * f = (fifo *) fd;
	while(f->writes == 0) {
		printf("Waiting for write\n");
		return SYSR_BLOCK;
	}
	for(; f->buf_rd_i < read_size && (f->writes > 0 || f->write_locked); f->buf_rd_i++, f->rd_i++)
	{
		if(f->writes == 0) {
			printf("Waiting for more...\n");
			return SYSR_BLOCK;
		}
		
		if(f->rd_i == FIFO_DATA_SIZE) {
			f->rd_i = 0;
		}
		buffer[f->buf_rd_i] = f->data[f->rd_i];
		f->writes--;
	}
	int i = f->buf_rd_i;
	f->buf_rd_i = 0;
	return i;
}

int fifo_close(int fd){
	// TODO;
	return 1;
}