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

int dirty_cache = 1;

int _cache = 1;

int disposed_history_reads[HDD_DISPOSE_HIST_COUNT];
int disposed_history[HDD_DISPOSE_HIST_COUNT];
int disposed_history_index = 0;

int hdd_dispose(int block_id) {
	if(!kernel_rd())	{
		return 1;
	}

	if(disposed_history[disposed_history_index] == block_id)	{
		return 1;
	}
	disposed_history[disposed_history_index++] = block_id;
	if(disposed_history_index == HDD_DISPOSE_HIST_COUNT - 1)	{
		disposed_history_index = 0;
	}
	return 1;
}

int hdd_disposed_reads(int block_id) {
	if(!kernel_rd())	{
		return 0;
	}
	
	int count = 0;
	int i = 0;
	for(i = 0; i < HDD_DISPOSE_HIST_COUNT; ++i)	{
		count += (disposed_history[i] == block_id);
	}
	return count;
}

int least_reads_family_index() {
	int i = 0;
	int j = 0;
	int min_reads = 100000;
	int min_reads_index = -1;
	for (; i < HDD_READ_GROUP_COUNT; i++) {
		if(cache.metadata[i * HDD_READ_GROUP_SIZE].untouchable)	{
			continue;
		}
		
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
		}
	}

	if (min_reads_index == -1) {
		min_reads = 100000;
		for (i = 0; i < HDD_READ_GROUP_COUNT; i++) {
			if(cache.metadata[i * HDD_READ_GROUP_SIZE].untouchable)	{
				continue;
			}
			int reads = 0;
			int writes = 0;
			for (j = 0; j < HDD_READ_GROUP_SIZE; j++) {
				reads += cache.metadata[i * HDD_READ_GROUP_SIZE + j].reads;
				writes += cache.metadata[i * HDD_READ_GROUP_SIZE + j].writes;
			}
			if (!writes && reads < min_reads && !hdd_disposed_reads(cache.metadata[i * HDD_READ_GROUP_SIZE].block)) {
				min_reads = reads;
				min_reads_index = i;
			}
		}
	}
	if (min_reads_index == -1) {
		hdd_cache_sync(TRUE);
		min_reads = 100000;
		for (i = 0; i < HDD_READ_GROUP_COUNT; i++) {
			if(cache.metadata[i * HDD_READ_GROUP_SIZE].untouchable)	{
				continue;
			}
			
			int reads = 0;
			for (j = 0; j < HDD_READ_GROUP_SIZE; j++) {
				reads += cache.metadata[i * HDD_READ_GROUP_SIZE + j].reads;
			}
			if (reads < min_reads && !hdd_disposed_reads(cache.metadata[i * HDD_READ_GROUP_SIZE].block)) {
				min_reads = reads;
				min_reads_index = i;
			}
		}
	}
	
	if (min_reads_index == -1) {
		min_reads = 100000;
		for (i = 0; i < HDD_READ_GROUP_COUNT; i++) {
			if(cache.metadata[i * HDD_READ_GROUP_SIZE].untouchable)	{
				continue;
			}
			
			int reads = 0;
			for (j = 0; j < HDD_READ_GROUP_SIZE; j++) {
				reads += cache.metadata[i * HDD_READ_GROUP_SIZE + j].reads;
			}
			if (reads < min_reads) {
				min_reads = reads;
				min_reads_index = i;
			}
		}
	}
	hdd_dispose(cache.metadata[min_reads_index * HDD_READ_GROUP_SIZE].block);
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
		cache.metadata[least_reads_fam * HDD_READ_GROUP_SIZE + i].untouchable  = !kernel_rd();

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

	hdd_cache_add((void *)&buffer, HDD_READ_GROUP_SIZE, (block_id / HDD_READ_GROUP_SIZE) 
			* HDD_READ_GROUP_SIZE, write);
			
	i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			if (!write) {
				cache.metadata[i].reads++;
			}
			return &cache.data[i];
		}
	}

	return NULL;
}

int hdd_write_block(char * data, int block_id) {
	hdd_get_block(block_id, FALSE);
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
			dirty_cache = dirty_cache | cache.metadata[i].writes;
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
	if (nwrites > 8 || (flush_force && nwrites > 0)) {
		// printf("Making %d writes | %d\n", nwrites, flush_force);
		_disk_write(ATA0, (void *)&cache.data[family_id], 2 * HDD_READ_GROUP_SIZE, 
			cache.metadata[family_id].block * 2 + 1);
	}
	return !!nwrites;
}



int hdd_cache_sync(int full) {
	if(!dirty_cache)
	{
		return 0;
	}
	
	int i = 0;
	int count = 0;
	for (; i < HDD_READ_GROUP_SIZE; i++) {
		if (cache.metadata[i * HDD_READ_GROUP_SIZE].block != -1) {
			count += hdd_flush_family(i * HDD_READ_GROUP_SIZE, TRUE);
			if(count && !full)	{
				return;
			}
		}
	}
	dirty_cache = 0;
	// krn = 1;
	// printf("syncd %d\n", count);
	// krn = 0;
	return 1;
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
