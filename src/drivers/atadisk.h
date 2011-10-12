#ifndef __ATADISK_H__
#define __ATADISK_H__
//#include "smdio.h"
#include "../kernel/semaphore.h"


#define BLOCK_SIZE       512    /* physical sector size in bytes */

#define ATA_GET_CAPACITY  123

void init_atadisk_fd ( void );
static void _disk_read(int ata, char * ans, unsigned short sector, int offset, int count);
static void _disk_write(int ata, char * msg, int bytes, unsigned short sector, int offset);
static void identifyDevice( void );

#endif


