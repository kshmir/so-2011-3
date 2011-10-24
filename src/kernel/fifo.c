#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/list.h"

#include "scheduler.h"
#include "semaphore.h"
#include "fifo.h"
#include "kernel.h"

#define FIFO_DATA_SIZE 1024

typedef struct fifo {
	int inode;
	char data[FIFO_DATA_SIZE];
	int	wr_i;
	int rd_i;
	int buf_rd_i;
	int buf_wr_i;
	int write_locked;
	int writes;
	Process * wr_lck_p;
	Process * rd_lck_p;
} fifo;

static list fifos = NULL;

/* Searchs for the fifo containing the inode.*/
static fifo * fifo_find(int inode) {
	foreach(fifo *, f, fifos) { 
		if(inode == f->inode)	{
			return f;
		}
	}
	return NULL;
}

/* #TODO*/
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
		f->wr_lck_p     = NULL;
		f->rd_lck_p     = NULL;
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
/* Writes a message to the fifo's file descriptor. #TODO: complete*/
int fifo_write(int fd, char * msg, int len){ 
	fifo * f = (fifo *) fd;

	for(; f->buf_wr_i < len; f->buf_wr_i++, f->wr_i++)	{
		
		if(f->writes > 0 && f->rd_lck_p != NULL) {
			process_setready(f->rd_lck_p);
			f->rd_lck_p = NULL;
		}
		if(f->writes == FIFO_DATA_SIZE) {
			f->write_locked = 1;
			f->wr_lck_p = getp();
			getp()->state = PROCESS_BLOCKED;
			return SYSR_BLOCK;
		}
		f->write_locked = 0;
		if(f->wr_i == FIFO_DATA_SIZE) {
			f->wr_i = 0;
		}
		f->data[f->wr_i] = msg[f->buf_wr_i];
		f->writes++;
	}	
	f->buf_wr_i     = 0;
	if(f->rd_lck_p != NULL) {
		process_setready(f->rd_lck_p);
		f->rd_lck_p = NULL;
	}
//	printf("Written len %d\n", len);
}

int fifo_read(int fd, char * buffer, int read_size){	
	fifo * f = (fifo *) fd;
	while(f->writes == 0) {
		f->rd_lck_p = getp();
		getp()->state = PROCESS_BLOCKED;
		return SYSR_BLOCK;
	}
	for(; f->buf_rd_i < read_size && (f->writes > 0 || f->write_locked); f->buf_rd_i++, f->rd_i++)
	{
		if(f->writes > 0 && f->wr_lck_p != NULL) {
			process_setready(f->wr_lck_p);
			f->wr_lck_p = NULL;
		}
		else if(f->writes == 0 && f->buf_wr_i > 0) {
			f->rd_lck_p = getp();
			getp()->state = PROCESS_BLOCKED;
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