/*
 *  hdd.c
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

// Made for debugging purposes, used as RAM on OSX, and as a wrapper in real life, probably used for cache later on.


#include "hdd.h"
#include "fs.h"


void hdd_init() {
	// Nothing to do on this end.
}

void hdd_read(char * answer, unsigned int sector) {
	_disk_read(ATA0, answer, 2, sector);
}

void hdd_write(char * buffer, unsigned int sector) {
	_disk_write(ATA0, buffer, 2, sector);
}

void hdd_close() {
	// Nothing to do on this end.
}
