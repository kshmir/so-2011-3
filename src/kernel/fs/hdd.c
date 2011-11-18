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

extern int krn;

hdd * hd;

hdd_cache cache;

hdd_block buffer[HDD_READ_GROUP_SIZE];

int _cache = 1;

int least_reads_family_index() {
	int i = 0;
	int j = 0;
	int min_reads = 100000;
	int min_reads_index = -1;
	for (; i < HDD_READ_GROUP_COUNT; i++) {
		int reads = 0;

		int valid = 0;
		for (j = 0; j < HDD_READ_GROUP_SIZE; j++) {
			if (cache.metadata[i * HDD_READ_GROUP_SIZE + j].block == -1) {
				valid = 1;
				reads += cache.metadata[i * HDD_READ_GROUP_SIZE + j].reads;
			}
		}
		if (reads < min_reads && valid) {
			min_reads = reads;
			min_reads_index = i;
			if (!reads) {
				break;
			}
		}
	}

	if (min_reads_index == -1) {
		printf("i must clear cache\n");
		hdd_cache_sync();
		for (i = 0; i < HDD_READ_GROUP_COUNT; i++) {
			int reads = 0;
			for (j = 0; j < HDD_READ_GROUP_SIZE; j++) {
				reads += cache.metadata[i * HDD_READ_GROUP_SIZE + j].reads;
			}
			if (reads < min_reads) {
				min_reads = reads;
				min_reads_index = i;
				if (!reads) {
					printf("I find shit\n");
					return min_reads_index;
					break;
				}
			}
		}
		printf("I dont find anything :(\n");
	}

	return min_reads_index;
}

int hdd_cache_add(hdd_block * buff, int len, int start_block, int write) {
	int least_reads_fam = least_reads_family_index();

	hdd_flush_family(least_reads_fam * HDD_READ_GROUP_SIZE, FALSE);

	int i = 0;
	int j = 0;	
	for (; i < len; i++) {
		cache.metadata[least_reads_fam * HDD_READ_GROUP_SIZE + i].block   = start_block + i;
		cache.metadata[least_reads_fam * HDD_READ_GROUP_SIZE + i].reads   = 0;
		cache.metadata[least_reads_fam * HDD_READ_GROUP_SIZE + i].writes  = 0;

		for (j = 0; j < HDD_BLOCK_SIZE; j++) {
			cache.data[least_reads_fam * HDD_READ_GROUP_SIZE + i].data[j] = buff[i].data[j];
		}
	}
}

void * hdd_get_block(int block_id, int write) {
	
	int i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			if (!write) {
				cache.metadata[i].reads++;
			}
			return &cache.data[i];
		}
	}
	
	_disk_read(ATA0, (void*)&buffer, 2 * HDD_READ_GROUP_SIZE, 
		(block_id / HDD_READ_GROUP_SIZE) * HDD_READ_GROUP_SIZE * 2 + 1);

	printf("Adding to cache\n");
	hdd_cache_add((void *)&buffer, HDD_READ_GROUP_SIZE, (block_id / HDD_READ_GROUP_SIZE) 
			* HDD_READ_GROUP_SIZE, write);
			
	printf("Added to cache\n");	
	i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			if (!write) {
				cache.metadata[i].reads++;
			}
			printf("found da shit\n");	
			return &cache.data[i];
		}
	}

	printf("NOT FOUND SHIT\n");	
	return NULL;
}

int hdd_write_block(char * data, int block_id) {
	hdd_get_block(block_id, TRUE);
	int i = 0;
	int j = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			int changes = 0;
			for (j = 0; j < HDD_BLOCK_SIZE; j++) {
				changes = changes | cache.data[i].data[j] != data[j];
				cache.data[i].data[j] = data[j];
			}
			cache.metadata[i].writes += !!changes;
			return 1;
		}
	}	
	return 0;
}


int hdd_flush_family(int family_id, int flush_force) {
	int i = 0;
	int nwrites = 0;
	for (; i < HDD_READ_GROUP_SIZE; i++) {
		nwrites += cache.metadata[family_id + i].writes;
		cache.metadata[family_id + i].writes = 0;
	}
	if (nwrites) {
		printf("before sync\n");
		_disk_write(ATA0, (void *)&cache.data[family_id], 2 * HDD_READ_GROUP_SIZE, 
			cache.metadata[family_id].block * 2 + 1);
		printf("afte sync\n");
	}
	return !!nwrites;
}



int hdd_cache_sync() {
	int i = 0;
	int count = 0;
	for (; i < HDD_READ_GROUP_SIZE; i++) {
		if (cache.metadata[i * HDD_READ_GROUP_SIZE].block != -1) {
			count += hdd_flush_family(i * HDD_READ_GROUP_SIZE, TRUE);
		}
	}
	krn = 1;
	printf("syncd %d\n", count);
	krn = 0;
}



void hdd_init() {

	int i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		cache.metadata[i].block  = -1;
		cache.metadata[i].reads  = 0;
		cache.metadata[i].writes = 0;
	}
}



void hdd_read(char * answer, unsigned int sector) {
	if (!sector) {
		return;
	}

	if (_cache) {
		char * data = hdd_get_block((sector - 1) / 2, FALSE);
		int i = 0;
		for (; i < HDD_BLOCK_SIZE; i++) {
			answer[i] = data[i];
		}	
	} else {
		_disk_read(ATA0, answer, 2, sector);
	}
}

void hdd_write(char * buffer, unsigned int sector) {
	if (!sector) {
		return;
	}

	if (_cache) {
		hdd_write_block(buffer, (sector - 1) / 2);
	} else {
		_disk_write(ATA0, buffer, 2, sector);
	}
}

void hdd_close() {
	// Nothing to do on this end.
}
