#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"
#include "../libs/list.h"

#include "scheduler.h"
#include "semaphore.h"
#include "fifo.h"
#include "kernel.h"

#define FIFO_DATA_SIZE 1024


// Data structure for handling the FIFO
typedef struct fifo {
	int key;							// Fifo key
	char data[FIFO_DATA_SIZE];			// Buffer
	int	wr_i;							// Index of where we're writing.
	int rd_i;							// Index of where we're reading.
	int buf_rd_i;						// Index of where in the data we're reading.
	int buf_wr_i;						// Index of where in the data we're writing.
	int write_locked;					// Flag for locking.
	int writes;							// Tells the amount of writes, for locking.
	Process * wr_lck_p;					// Process locked in the fifo for writing.
	Process * rd_lck_p;					// Process locked in the fifo for reading.
} fifo;

static list fifos = NULL;

/* Searchs for the fifo containing the key.*/
static fifo * fifo_find(int key) {
	foreach(fifo *, f, fifos) { 
		if(key == f->key)	{
			return f;
		}
	}
	return NULL;
}

// Tells if a fifo exists
int fifo_exists(int file_name) {
	return (int)fifo_find(file_name);
}


//  Builds a fifo.
int fifo_make(int ptr) {
	if(fifos == NULL)	{
		fifos = list_init();
	}
	
	int n;
	int i;
	n = ptr;
	
	fifo * f = NULL;
	if((f = fifo_find(n)) == NULL)
	{
		f               = (fifo *) malloc(sizeof(fifo));
		f->key          = n;
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
}

// Reads a fifo, it might block.
int fifo_read(int fd, char * buffer, int read_size){	
	fifo * f = (fifo *) fd;
	while(f->writes == 0) {										// No writes? Then block!
		f->rd_lck_p = getp();

		getp()->state = PROCESS_BLOCKED;
		return SYSR_BLOCK;
	}
	for(; f->buf_rd_i < read_size && (f->writes > 0 || f->write_locked); f->buf_rd_i++, f->rd_i++)
	{
		if(f->writes > 0 && f->wr_lck_p != NULL) {
			process_setready(f->wr_lck_p);						// If there's a write and it's blocked, unlock the writer
			f->wr_lck_p = NULL;
		}
		else if(f->writes == 0 && f->buf_wr_i > 0) {
			f->rd_lck_p = getp();
			getp()->state = PROCESS_BLOCKED;
			return SYSR_BLOCK;									// If we've got no writes, then we must block, just like before.
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

// Closes a fifo, formality :p
int fifo_close(int fd){
	fifo * f = (fifo *) fd;
	if(f->wr_lck_p != NULL) {
		process_setready(f->wr_lck_p);
	}
	if(f->rd_lck_p != NULL) {
		process_setready(f->rd_lck_p);
	}
	f->wr_i         = 0;
	f->rd_i         = 0;
	f->writes       = 0;
	f->write_locked = 0;
	f->buf_wr_i     = 0;
	f->buf_rd_i     = 0;
	f->wr_lck_p     = NULL;
	f->rd_lck_p     = NULL;
	int i = 0;
	for(; i < FIFO_DATA_SIZE; ++i)	{
		f->data[i] = 0;
	}
	return 1;
}