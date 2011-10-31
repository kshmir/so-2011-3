/*
 *  hdd.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */
#include "../../drivers/atadisk.h"

#ifndef _HDD_H_
#define _HDD_H_

#define SECTOR_SIZE 512

void hdd_init();

void hdd_read(char * answer, unsigned int sector);

void hdd_write(char * buffer, unsigned int sector);

void hdd_close();

#endif