/*
 *  fs.c
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

#include "fs.h"
#include "hdd.h"
#include "bitmap.h"
#include "../tty.h"

#ifdef __MACH__
	#include <stdio.h>
	#include <string.h>
#else 
	#include "../../libs/stdio.h"
	#include "../../libs/string.h" 
#endif

#define _FS_DEBUG 0



// Where all the magic actually happens :D
static fs_data dat;
static super_block * sb = NULL;
static group_descriptor * gbdt = NULL;
static block g_inode_bmps[FS_BLOCK_GROUP_COUNT];
static block g_block_bmps[FS_BLOCK_GROUP_COUNT];
static bitmap * bm_inodes;
static bitmap * bm_blocks;



// For use in IO
static block b; // Block containing data.
static block bi; // Block containing indirects.
static inode n;
static int dir_op_offset;
static int f_op_offset;
static dir_entry * dot;
static dir_entry * _old_dot;
static int * indirect_block_data;
static int fs_done;



///////////// Block Handling

static void block_write(void * data, unsigned int block_n) {
	// printf("Start write\n");
	hdd_write( data, block_n * 2 + 1);	
//	hdd_write( data + SECTOR_SIZE, block_n * 2 + 2);	

}

static void block_read(void * data, unsigned int block_n) {
	hdd_read( data, block_n * 2 + 1);	
//	hdd_read( data + SECTOR_SIZE, block_n * 2 + 2);		

}

static void block_clear(block * b){
	int i = 0;
	for(; i < FS_BLOCK_SIZE / sizeof(int); i++) {
		((int *)b)[i] = 0;
	}
}


//////////// MBR

void fs_mbr_read() {
	block_read((char *)&dat.head._start               ,0);
}

//////////// Superblock

void fs_sb_init() {
	
	// Blocks
	sb->s_blocks_per_group = FS_DATA_TABLE_SIZE;
	sb->s_frags_per_group  = sb->s_blocks_per_group;
	sb->s_blocks_count =     FS_BLOCK_GROUP_COUNT * sb->s_blocks_per_group;
	sb->s_free_blocks_count = sb->s_blocks_count;
	sb->s_r_blocks_count = 0;
	
	// Inodes
	sb->s_inodes_per_group  = FS_INODE_TABLE_SIZE  * sizeof(block) / sizeof(inode);
	sb->s_inodes_count =      FS_BLOCK_GROUP_COUNT * sb->s_inodes_per_group;
	sb->s_free_inodes_count = sb->s_inodes_count;
	
	sb->s_log_block_size	= FS_BLOCK_SIZE;
	sb->s_log_frag_size		= FS_BLOCK_SIZE;	// Mantains compatibility
	
	sb->s_first_data_block = 0;
	
	sb->s_magic	= 0xEF53;		// ext2's magic number.
}

void fs_sb_read() {
	block_read( (char *)&dat.head._super               ,1);
}

void fs_sb_write() {
	block_write((char *)&dat.head._super               ,1);
}

//////////// GBDT

static void fs_gbdt_init() {
	int i = 0;
	for (; i < FS_BLOCK_GROUP_COUNT; i++) {
		gbdt[i].bg_block_bitmap       = i * FS_BLOCK_GROUP_SIZE + FS_GLOB_GB_OFFSET;
		gbdt[i].bg_inode_bitmap       = i * FS_BLOCK_GROUP_SIZE + FS_GLOB_GB_OFFSET + 1;
		gbdt[i].bg_inode_table        = i * FS_BLOCK_GROUP_SIZE + FS_GLOB_GB_OFFSET + 2;
		gbdt[i].bg_free_inodes_count  = FS_INODE_TABLE_SIZE * sizeof(block) / sizeof(inode);
		gbdt[i].bg_free_blocks_count  = FS_DATA_TABLE_SIZE;
		gbdt[i].bg_used_dirs_count    = 0;
	}
}

static void fs_gbdt_read() {
	block_read( (char *)&dat.head._descriptors               ,2);
}

static void fs_gbdt_write() {
	block_write((char *)&dat.head._descriptors               ,2);
}

///////////// Bitmaps Cache

static void fs_bitmaps_init() {
	int i = 0;
	for (; i < FS_BLOCK_GROUP_COUNT; i++) {
		block_read((void *)&g_inode_bmps[i], gbdt[i].bg_inode_bitmap);
		block_read((void *)&g_block_bmps[i], gbdt[i].bg_block_bitmap);
	}
	
	
	
	bm_inodes = bitmap_init(FS_INODE_TABLE_SIZE * FS_INODES_PER_BLOCK, FS_BLOCK_SIZE * 8, g_inode_bmps);
	bm_blocks = bitmap_init(FS_DATA_TABLE_SIZE, FS_BLOCK_SIZE * 8, g_block_bmps);
}

static void fs_bitmaps_write_all() {
	int i = 0;
	for (; i < FS_BLOCK_GROUP_COUNT; i++) {
		block_write((void *)&g_inode_bmps[i], gbdt[i].bg_inode_bitmap);
		block_write((void *)&g_block_bmps[i], gbdt[i].bg_block_bitmap);
	}	
}

static void fs_bitmaps_write(int i) {
	block_write((void *)&g_inode_bmps[i], gbdt[i].bg_inode_bitmap);
	block_write((void *)&g_block_bmps[i], gbdt[i].bg_block_bitmap);	
}

//////////// Block group

static unsigned int bgroup_get(unsigned int index) {	
	return FS_GLOB_GB_OFFSET + index * FS_BLOCK_GROUP_SIZE / FS_BLOCK_SIZE;
}

static unsigned int bgroup_blockbmp(unsigned int index) {
	return index;
}

static unsigned int bgroup_inodebmp(unsigned int index) {
	return index + 1;
}

static unsigned int bgroup_inodetbl(unsigned int index) {
	return index + 2;
}

static unsigned int bgroup_blocktbl(unsigned int index) {
	return index + 2 + FS_INODE_TABLE_SIZE;
}




//////////// Data Block Handling 

// Gets the real block location of a data_block inside the disk.
// This allows all data to be treated as an array.
static unsigned int data_get_block(unsigned int block_n) {
	unsigned int block_group		= (block_n - 1) / sb->s_blocks_per_group; 
	unsigned int locd				= (block_n - 1) % sb->s_blocks_per_group; 
	unsigned int locd_block_i		= locd % FS_DATA_TABLE_SIZE;			  
	return bgroup_blocktbl(bgroup_get(block_group)) + locd_block_i; 	
}

static void data_read_block(void * data, unsigned int block_n) {
	unsigned int block_in_disk		= data_get_block(block_n);
	block_read(data, block_in_disk);
}

static void data_write_block(void * data, unsigned int block_n) {			  
	
	unsigned int block_group		= (block_n - 1) / sb->s_blocks_per_group; 
	unsigned int block_in_disk		= data_get_block(block_n);
	
	// Write the data in the disk
	block_write(data, block_in_disk);											
	
	// Write the bitmap in the disk
	
	bitmap_write(bm_blocks, block_n - 1, 1);
	block_write(&g_block_bmps[block_group], bgroup_blockbmp(bgroup_get(block_group)));
	
	// The data is only persisted after it's saved in the bitmap.
}

static void data_release_block(unsigned int block_n) {			  
	unsigned int block_group		= (block_n - 1) / sb->s_blocks_per_group; 
	
	// Write the bitmap in the disk
	bitmap_write(bm_blocks, block_n - 1, 0);
	block_write(&g_block_bmps[block_group], bgroup_blockbmp(bgroup_get(block_group)));
}

///////////// Inode Handling 


static void inode_clear(inode * n){
	int i = 0;
	for(; i < FS_INODE_SIZE / sizeof(int); i++) {
		((int *)n)[i] = 0;
	}
	return;
}

// Gets the physical block of an inode
static unsigned int inode_get_block(unsigned int inode_n) {
	unsigned int block_group		= (inode_n - 1) / sb->s_inodes_per_group; // Global Block index of the inode
	unsigned int locin				= (inode_n - 1) % sb->s_inodes_per_group; // Absolute offset of the inode.
	unsigned int locin_block_i		= locin / FS_INODES_PER_BLOCK;			  // Block index of the inode inside the group.
	return bgroup_inodetbl(bgroup_get(block_group)) + locin_block_i; 
}

// Gets the index of an inode inside it's physical block;
static unsigned int inode_get_block_index(unsigned int inode_n) {
	unsigned int locin				= (inode_n - 1) % sb->s_inodes_per_group; // Absolute offset of the inode.
	return locin % FS_INODES_PER_BLOCK;									  // Index of the inode inside it's block.
}

// Writes an inode to disk.
static void inode_write(unsigned int inode_n, inode * n){
	unsigned int locin_block_index	= inode_get_block_index(inode_n);
	unsigned int real_block_index	= inode_get_block(inode_n);
	
	block b;
	block_clear(&b);
	block_read((void *)&b, real_block_index);

	inode * inodes = (inode *) &b;
	
	int i = 0;
	for (; i < FS_INODE_SIZE / sizeof(int); i++) {
		((int *)&inodes[locin_block_index])[i] = ((int*)n)[i];
	}
	block_write((void *)&b, real_block_index);
}

static void inode_read(unsigned int inode_n, inode * n) {
	unsigned int locin_block_index	= inode_get_block_index(inode_n);
	unsigned int real_block_index	= inode_get_block(inode_n);



	block_clear(&b);
	

	block_read((void *)&b, real_block_index);
	


	int i = 0;
	inode * inodes = (inode *) &b;
	
	for (; i < FS_INODE_SIZE / sizeof(int); i++) {
		((int*)n)[i] = ((int *)&inodes[locin_block_index])[i];
	}
}

// Gets the level of indirections needed for a given logical block
static int inode_get_indir_level(int log_block) {
	if (log_block < 12) {												// Direct blocks     | i -> Data
		return 0;
	} else if (log_block < 12 + 256) {									// Indirect blocks 1 | i -> j -> Data
		return 1;
	} else if (log_block < 12 + 256 + 256 * 256) {                       // Indirect blocks 2 | i -> j -> k -> Data
		return 2;		
	} else if (log_block < 12 + 256 + 256 * 256 + 256 * 256 * 256) {     // Indirect blocks 3 | i -> j -> k -> h -> Data
		return 3;
	}
	return -1; // If it's -1, then it's over the file size limit (16GB, WOOOO!)
}

static int inode_get_dir_block(int log_bock) {							// Gets 'i'
	if (log_bock < 12) {												// Direct blocks     | i -> Data
		return log_bock;
	} else if (log_bock < 12 + 256) {									// Indirect blocks 1 | i -> j -> Data
		return 12;
	} else if (log_bock < 12 + 256 + 256 * 256) {                       // Indirect blocks 2 | i -> j -> k -> Data
		return 13;		
	} else if (log_bock < 12 + 256 + 256 * 256 + 256 * 256 * 256) {     // Indirect blocks 3 | i -> j -> k -> h -> Data
		return 14; 
	}
	return -1; // If it's -1, then it's over the file size limit (16GB, WOOOO!)
}

static int inode_get_1indir_block(int log_block) {						// Gets 'j'	
	if (log_block < 12 + 256) {											// Indirect blocks 1 | i -> j -> Data
		return (log_block - 12) % 256;
	} else if (log_block < 12 + 256 + 256 * 256) {						// Indirect blocks 2 | i -> j -> k -> Data
		return ((log_block - 12 - 256) / 256);
	} else if (log_block < 12 + 256 + 256 * 256 + 256 * 256 * 256) {	// Indirect blocks 3 | i -> j -> k -> h -> Data
		return (log_block - 12 - 256 - 256 * 256) / (256 * 256); 
	}
	return -1; // If it's -1, then it's over the file size limit (16GB, WOOOO!)
}

static int inode_get_2indir_block(int log_block) {						// Gets 'k'	
	if (log_block < 12 + 256 + 256 * 256) {								// Indirect blocks 2 | i -> j -> k -> Data
		return ((log_block - 12 - 256) % 256);
	} else if (log_block < 12 + 256 + 256 * 256 + 256 * 256 * 256) {	// Indirect blocks 3 | i -> j -> k -> h -> Data
		return ((log_block - 12 - 256 - 256 * 256) % (256 * 256)) / 256; 
	}
	return -1; // If it's -1, then it's over the file size limit (16GB, WOOOO!)
}

static int inode_get_3indir_block(int log_block) {						// Gets 'h'
	if (log_block < 12 + 256 + 256 * 256 + 256 * 256 * 256) {			// Indirect blocks 3 | i -> j -> k -> h -> Data
		return (log_block - 12 - 256 - 256 * 256) % 256; 
	}
	return -1; // If it's -1, then it's over the file size limit (16GB, WOOOO!)
}


void make_ph_block(inode * n, int * ph_block, int * log_block) {
	int indirects = inode_get_indir_level(*log_block);
	
	int old_ph = * ph_block;
	int dirty = 0;
	
 	* ph_block = n->data_blocks[inode_get_dir_block(*log_block)];
	// Gets the next blocks's direction.
	if(!* ph_block) {
		* ph_block = bitmap_first_valued(bm_blocks, FS_DATA_BITMAP_SIZE, 0) + 1;
		bitmap_write(bm_blocks, (* ph_block) - 1, 1);
		n->data_blocks[inode_get_dir_block(*log_block)] = * ph_block;
		dirty = 1;
	} 
	
	if (indirects > 0) {
		block_clear(&bi);
		if (!dirty) {
			data_read_block(&bi, * ph_block);
		}
		
		dirty = 0;
			
		
		
		indirect_block_data = (int *) &bi;
		old_ph = * ph_block;	
		* ph_block = indirect_block_data[inode_get_1indir_block(*log_block)];
		// Gets the next blocks's direction.
		if(!* ph_block) {
			bitmap_write(bm_blocks, old_ph - 1, 1);
			* ph_block = bitmap_first_valued(bm_blocks, FS_DATA_BITMAP_SIZE, 0) + 1;
			bitmap_write(bm_blocks, (* ph_block) - 1, 1);
			indirect_block_data[inode_get_1indir_block(*log_block)] = * ph_block;
			data_write_block(&bi, old_ph);
			dirty = 1;
		} 
		if (indirects > 1) {
			block_clear(&bi);
			if (!dirty) {
				data_read_block(&bi, * ph_block);
			}
			
			dirty = 0;
			old_ph = * ph_block;		
			* ph_block = indirect_block_data[inode_get_2indir_block(*log_block)];
			
			// Gets the next blocks's direction.
			if(!* ph_block) {
				bitmap_write(bm_blocks, old_ph - 1, 1);
				* ph_block = bitmap_first_valued(bm_blocks, FS_DATA_BITMAP_SIZE, 0) + 1;
				bitmap_write(bm_blocks, (* ph_block) - 1, 1);
				indirect_block_data[inode_get_2indir_block(*log_block)] = * ph_block;
				data_write_block(&bi, old_ph);
				dirty = 1;
			} 
			
			if (indirects > 2) {
				block_clear(&bi);
				if (!dirty) {
					data_read_block(&bi, * ph_block);
				}
				
				dirty = 0;
				old_ph = * ph_block;		
				* ph_block = indirect_block_data[inode_get_3indir_block(*log_block)];
				
				// Gets the next blocks's direction.
				if(!* ph_block) {
					bitmap_write(bm_blocks, old_ph - 1, 1);
					* ph_block = bitmap_first_valued(bm_blocks, FS_DATA_BITMAP_SIZE, 0) + 1;
					bitmap_write(bm_blocks, (* ph_block) - 1, 1);
					indirect_block_data[inode_get_3indir_block(*log_block)] = * ph_block;
					data_write_block(&bi, old_ph);
				} 
			}
		}
	}	
}

void log_block_write(inode * n, int * log_block) {
	int ph_block;	
	
	make_ph_block(n, &ph_block, log_block);
	
	// Write the data inside the data block
	bitmap_write(bm_blocks, ph_block - 1, 1);
	data_write_block(&b, ph_block);	
}

void log_block_read(inode * n, int * log_block) {
	int ph_block;
	
	make_ph_block(n, &ph_block, log_block);
	
	// Write the data inside the data block
	bitmap_write(bm_blocks, ph_block - 1, 1);
	data_read_block(&b, ph_block);	
}

// When a buffer is full, writes the block and gets the next one.
void add_block(inode * n, int * log_block) {
	log_block_write(n, log_block);
	
	(*log_block)++;	
	
	log_block_read(n, log_block);

	dir_op_offset = 0;
	dot = (dir_entry *) ((char*)&b + dir_op_offset);	
}

int add_dir_entry(inode * n, int file_type, int inode, char * name, int * log_block) {
	int ret = 0;
	
	dot = (dir_entry *) ((char*)&b + dir_op_offset);
	
	while (dot->inode || (dot->name_len == 0 && dot->inode)) {
		dir_op_offset += dot->rec_len;
		dot = (dir_entry *) ((char*)&b + dir_op_offset);
	}
	
	
	_old_dot = dot;
	if (dir_op_offset + 8 + strlen(name) + 4 - strlen(name) % 4 >= FS_BLOCK_SIZE) { // Oooops, next one!
		int old_off = (int) _old_dot - (int) &b;
		_old_dot->rec_len = old_off;
		add_block(n, log_block);
	}
	
	dot->inode = inode;
	dot->file_type = file_type;
	dot->name_len = strlen(name);
	dot->rec_len = 8 + dot->name_len + 4 - dot->name_len % 4;
	dir_op_offset += dot->rec_len;
	
	int i = 0;
	for (; i < dot->name_len; i++) {
		dot->name[i] = name[i];
	}
	return ret;
}	

dir_entry * iterate_dir_entry(block * b) {
	if (dir_op_offset > FS_BLOCK_SIZE) {
		return NULL;
	}
	
	dot = (dir_entry *) ((char*)b + dir_op_offset);
	dir_op_offset += dot->rec_len;
	if (dot->rec_len == 0) {
		return NULL;
	}
	return dot;
}

/// Filesystem Initialization
void fs_init() {
	
	int i = 0;
	for (; i < sizeof(fs_data) / 4; i += 4) {
		((int*)&dat)[i] = 0;
	}
	
	fs_sb_read();

	
	sb = (super_block *) &dat.head._super;
	gbdt = (group_descriptor *) &dat.head._descriptors;
	
	// If the magic number is on then the fs is already loaded.
	if(sb->s_magic == 0xEF53) {
		fs_gbdt_read();
		fs_bitmaps_init();
		if (_FS_DEBUG) {
			printf("SUPERBLOCK DEBUGGING\n");
			printf("\tFree Blocks: %d\t Total Blocks: %d\n", 
				   bitmap_block_count(bm_blocks, FS_DATA_BITMAP_SIZE, 0),
				   FS_DATA_BITMAP_SIZE);
			printf("\tFree Inodes: %d\t Total Inodes: %d\n",
				   bitmap_block_count(bm_inodes, FS_INODE_BITMAP_SIZE, 0),
				   FS_INODE_BITMAP_SIZE);

			
			
			inode_read(1, &n);
		
			
			int index = 0;
			int max_log_block = n.blocks / 2;
			if (!max_log_block) {
				max_log_block = 1;
			}
			dir_op_offset = 0;
			block_clear(&b);
			while (index < max_log_block + 1) {
				dir_entry * dot = iterate_dir_entry(&b);
				
				if (dot == NULL) {
					if (index == max_log_block) {
						break;
					}
					log_block_read(&n,&index);
					dir_op_offset = 0;
					index++;	
				} else {
					
					int len = dot->name_len, i = 0;
					for (; i < len; i++) {
						printf("%c", dot->name[i]);
					}
					printf("\n");
				}
			}
			
		}
		fs_bitmaps_write_all();
		fs_gbdt_write();
		fs_sb_write();
	}
	else {
		printf("Preparing to start FS\n");
		
		printf("Starting SB\n");
		fs_sb_init();	
		printf("Starting GBDT\n");
		fs_gbdt_init();	
		printf("Starting Bitmaps\n");
		fs_bitmaps_init();
		
		printf("Creating /\n");
		int slash_inode = fs_mkdir("/", 1);
		printf("Creating /dev\n");
		int dev_inode   = fs_mkdir("dev", slash_inode);
		printf("Creating /home\n");
		int home_inode  = fs_mkdir("home", slash_inode);
		printf("Creating /root\n");
		int root_inode  = fs_mkdir("root", slash_inode);
		printf("Creating /etc\n");
		int etc_inode   = fs_mkdir("etc", slash_inode);
		printf("Creating /tmp\n");
		int tmp_inode   = fs_mkdir("tmp", slash_inode);
		printf("Creating /tmp/testfile\n");
		int f1 = fs_open_reg_file("testfile", tmp_inode, 0);
		
		printf("FS ended...\n");
		
		printf("\tFree Blocks: %d\t Total Blocks: %d\n", 
			   bitmap_block_count(bm_blocks, FS_DATA_BITMAP_SIZE, 0),
			   FS_DATA_BITMAP_SIZE);
		printf("\tFree Inodes: %d\t Total Inodes: %d\n",
			   bitmap_block_count(bm_inodes, FS_INODE_BITMAP_SIZE, 0),
			   FS_INODE_BITMAP_SIZE);
		
		fs_bitmaps_write_all();
		fs_gbdt_write();
		fs_sb_write();
	}
}

void fs_finish() {
	fs_done = 1;
}

unsigned int fs_read_file(int inode, char * data, int size, unsigned long * f_offset) {
	inode_clear(&n);
	
	inode_read(inode, &n);
	int log_block = * f_offset / FS_BLOCK_SIZE;
	int block_index = * f_offset % FS_BLOCK_SIZE;
	int top_log_block = n.blocks / 2;
	int i = 0;
	
	if (!top_log_block && n.mode & EXT2_S_IFDIR) {
		top_log_block = 1;
	}

	if (log_block < top_log_block || log_block == top_log_block && block_index <= n._last_write_offset) {

		log_block_read(&n, &log_block);
		char * block = (char *) &b;
		for (; i < size; i++, block_index++) {
			data[i] = block[block_index];
			
			if (top_log_block == log_block && block_index == n._last_write_offset) {
				break;
			}
			
			if (block_index == FS_BLOCK_SIZE - 1) {
				log_block++;
				log_block_read(&n, &log_block);
				block_index = -1;
			}
		}
		* f_offset += i;
	} else {
		return 0; // Bad Offsetttttt!!!!
	}

	
	return i;
}

unsigned int fs_write_file(int inode, char * data, int size) {
	inode_clear(&n);
	inode_read(inode, &n);
	

	int log_block = n.blocks / 2;
	block_clear(&b);
	
	int i = 0;
	int block_index = n._last_write_offset;
	log_block_read(&n, &log_block);
	char * block = (char *) &b;
	for (; i < size; i++,block_index++) {
		block[block_index] = data[i];
		if (block_index == FS_BLOCK_SIZE - 1) {
			log_block_write(&n, &log_block);
			log_block++;
			log_block_read(&n, &log_block);
			block_index = -1;
		}
	}
	n._last_write_offset = block_index;
	log_block_write(&n, &log_block);	
	
	n.blocks = log_block * 2;

	inode_write(inode,&n);

	return i;
}

void delete_internal_inodes(int log_block) {

	int indirects = inode_get_indir_level(log_block);
	
	if (indirects == 0) {
		bitmap_write(bm_blocks, n.data_blocks[log_block] - 1, 0);
	}
	
	int i1_delete = 1;
	int i2_delete = 1;
	int i3_delete = 1;
	
	int i1_ptr;
	int i2_ptr;
	int i3_ptr;
	
	int * data = (int *) &bi;
	block_clear(&bi);
	if (indirects >= 1) {
		data_read_block(&bi, (i1_ptr = n.data_blocks[inode_get_dir_block(log_block)]) );
		i1_delete = inode_get_1indir_block(log_block) == 0;
		if (indirects >= 2) {
			data_read_block(&bi, (i2_ptr = data[inode_get_1indir_block(log_block)]) );
			i2_delete = inode_get_2indir_block(log_block) == 0;

			if (indirects >= 3) {
				i3_ptr = data[inode_get_1indir_block(log_block)];
				i3_delete = inode_get_3indir_block(log_block) == 0;
				
				if (i3_delete) {
					bitmap_write(bm_blocks, i3_ptr - 1, 0);
				}
			}
			
			i2_delete = i2_delete && i3_delete;
			if (i2_delete) {
				bitmap_write(bm_blocks, i2_ptr - 1, 0);
			}
		}
		
		i1_delete = i1_delete && i2_delete;
		if (i1_delete) {
			bitmap_write(bm_blocks, i1_ptr - 1, 0);
			bitmap_write(bm_blocks, n.data_blocks[log_block] - 1, 0);
		}

	}	
}

unsigned int fs_rm(unsigned int inode, int recursive) {
	
	
	inode_read(inode, &n);

	// if (!recursive) { // I'm not sure about this, but well... at least it's just a security flaw.
	// 	inode_read(n._dir_inode, &n);
	// 	if(!fs_has_perms(&n, ACTION_WRITE))
	// 	{
	// 		return 0;
	// 	}
	// }
	

	int log_block = n.blocks / 2 + 1;
	int ph_block = 0;
	
	while(log_block > 0) {
		log_block--;
		make_ph_block(&n, &ph_block, &log_block);
	 	
		if (n.mode & EXT2_S_IFDIR) { // If it's a directory... then... RECURSION! D:
			block entries;
			data_read_block(&entries, ph_block);
			
			int off = 0;
			dir_op_offset = 0;
			dir_entry * old_dot = NULL;
			dir_entry * dot = iterate_dir_entry(&entries);
			// Iterates dir entries
			while (dot != NULL) {
				if (dot == NULL) {
					break;
				} else {
					if (dot->name_len > 0 && dot->inode != inode && dot->inode != n._dir_inode) {
						int _rm = fs_rm(dot->inode, 1);
						if(!_rm)
						{
							data_write_block(&entries, ph_block);
							return 0;
						}
						off = dir_op_offset;
						dot->name_len = 0;
						dir_op_offset = off;
						inode_read(inode, &n);
					}
				}
				old_dot = dot;
				dot = iterate_dir_entry(&entries);
			}	

			data_write_block(&entries, ph_block);
		}

		bitmap_write(bm_blocks, ph_block - 1, 0);
		delete_internal_inodes(log_block);
	}
	
	if (!recursive) {
		unsigned int folder_inode = n._dir_inode;
		inode_read(folder_inode, &n);		
		folder_rem_direntry(inode, folder_inode);
	}
	
	bitmap_write(bm_inodes, inode, 0);


	fs_bitmaps_write_all();  

						  
	return 1;
}

unsigned int fs_indir(char * name, int folder_inode) {
	inode_read(folder_inode, &n);
	
	int index = 0;
	int max_log_block = n.blocks / 2;
	if (!max_log_block) {
		max_log_block = 1;
	}
	dir_op_offset = 0;
	block_clear(&b);
	
	dir_entry * old_dot = NULL;
	dir_entry * dot = NULL;
	// Iterates dir entries
	while (index < max_log_block + 1) {
		old_dot = dot;
		dot = iterate_dir_entry(&b);
		if (dot == NULL) {
			if (index == max_log_block) {
				break;
			}
			log_block_read(&n,&index);
			dir_op_offset = 0;
			dot = NULL;
			index++;	
		} else {
			int i = 0; ;
			int name_len = strlen(name);
			if (name_len == dot->name_len) {
				int match = 1;
				for (;name[i] && match; i++) {
					match = name[i] == dot->name[i];
				}
				if (match) {
					return dot->inode;
				}
			}
		}
	}	
	return 0;
}

static char iname[255];
char * fs_iname(int inode, int folder_inode) {
	inode_read(folder_inode, &n);
	
	*iname = 0;
	
	int index = 0;
	int max_log_block = n.blocks / 2;
	if (!max_log_block) {
		max_log_block = 1;
	}
	dir_op_offset = 0;
	block_clear(&b);
	
	dir_entry * old_dot = NULL;
	dir_entry * dot = NULL;
	// Iterates dir entries
	while (index < max_log_block + 1) {
		old_dot = dot;
		dot = iterate_dir_entry(&b);
		if (dot == NULL) {
			if (index == max_log_block) {
				break;
			}
			log_block_read(&n,&index);
			dir_op_offset = 0;
			dot = NULL;
			index++;	
		} else {
			int i = 0; ;
			if (inode == dot->inode) {
				int match = 1;
				
				int i = 0;
				int len = dot->name_len;
				for(; i < len; ++i) {
					iname[i] = dot->name[i];
				}
				iname[i] = 0;
				return iname;
			}
		}
	}	
	return 0;
}

unsigned int folder_rem_direntry(unsigned int file_inode, unsigned int folder_inode) {
	inode_read(folder_inode, &n);
	
	int index = 0;
	int max_log_block = n.blocks / 2;
	if (!max_log_block) {
		max_log_block = 1;
	}
	dir_op_offset = 0;
	block_clear(&b);
	
	dir_entry * old_dot = NULL;
	dir_entry * dot = NULL;
	// Iterates dir entries
	while (index < max_log_block + 1) {
		old_dot = dot;
		dot = iterate_dir_entry(&b);
		if (dot == NULL) {
			if (index == max_log_block) {
				break;
			}
			log_block_read(&n,&index);
			dir_op_offset = 0;
			dot = NULL;
			index++;	
		} else {
			int i = 0; ;
			int match = file_inode == dot->inode;
			if (match) {
				if (old_dot != NULL) {
					old_dot->rec_len += dot->rec_len;
					dot->inode = 0;
					dot->name_len = 0; // Sets as deleted
				} else {
					dot->name_len = 0; // Sets as deleted
				}
				int to_write = index - 1;
				
				log_block_write(&n, &to_write);
			}
		}
	}	
	return 0;
}

unsigned int fs_open_link(char * name, unsigned int folder_inode, int mode) {
	return fs_open_file(name, folder_inode, mode, EXT2_S_IFLNK);
}

unsigned int fs_open_fifo(char * name, unsigned int folder_inode, int mode) {
	return fs_open_file(name, folder_inode, mode, EXT2_S_IFIFO);
}

unsigned int fs_open_reg_file(char * name, unsigned int folder_inode, int mode) {
	return fs_open_file(name, folder_inode, mode, EXT2_S_IFREG);
}

unsigned int fs_open_file(char * name, unsigned int folder_inode, int mode, int type) {
	unsigned int inode_id = 0;

	inode_id = fs_indir(name, folder_inode);
	if (inode_id) {
		if (mode & O_CREAT) {
			int _rm_res = fs_rm(inode_id, 0);
			if(!_rm_res) {
				return 0;
			}
			printf("O_CREAT delete\n");
		} else if(mode & O_NEW) {
			return 0;
		} else {
			int can = 1;
			// &n comes from the indir call up there... 
			// stackless is fun, but dangerous!
			if((mode & O_RD) && !fs_has_perms(&n, ACTION_READ))	{
				can = 0;
			}
			
			if((mode & O_WR) && !fs_has_perms(&n, ACTION_WRITE))	{
				can = 0;
			}
			
			if(can || !fs_done)
			{
				return inode_id;
			} else {
				return 0;
			}

		}
	}
	
	inode_id = bitmap_first_valued(bm_inodes, FS_INODE_BITMAP_SIZE, 0) + 1;
	
	printf("Got inode %d\n", inode_id);
	if (!folder_inode) {
		folder_inode = inode_id;
	}
	
	int log_block;
	
	if (folder_inode != inode_id) {
		inode_read(folder_inode, &n);
		
		if(!fs_has_perms(&n, ACTION_WRITE))	{
			return 0;
		}
		
		log_block = n.blocks / 2;
		
		block_clear(&b);
		dir_op_offset = 0;
		log_block_read(&n, &log_block);
		add_dir_entry(&n, EXT2_FT_DIR, inode_id, name, &log_block);
		log_block_write(&n, &log_block);
		
		inode_write(folder_inode, &n);	
	}
	bitmap_write(bm_inodes, inode_id - 1, 1);
		
	inode_clear(&n);
	
	
	if(!fs_done)
	{
		n.uid = 0;
	} else {
		n.uid = current_ttyc()->uid;
	}
	n.mode =  type;
	n.size = 0;
	n.blocks = 0; // Beware! This represents the SECTORS!
	
	log_block = n.blocks / 2;
	
	n.blocks = (log_block) * 2;
	
	n._last_write_offset = 0;
	n.i_file_acl = 511;
	n._dir_inode = folder_inode;
	
	inode_write(inode_id, &n);	
	
	inode_clear(&n);

	
	return inode_id;
}

unsigned int fs_mkdir(char * name, unsigned int parent_inode) {
	unsigned int inode_id = 0;
	
	if ((inode_id = fs_indir(name, parent_inode))) {
		return inode_id;
	}
	inode_id = bitmap_first_valued(bm_inodes, FS_INODE_BITMAP_SIZE, 0) + 1;
	
	if (!parent_inode) {
		parent_inode = inode_id;
	}
	
	int log_block;
	
	if (parent_inode != inode_id) {
		inode_read(parent_inode, &n);
		
		if(!fs_has_perms(&n, ACTION_WRITE))	{
			return 0;
		}
		
		
		log_block = n.blocks / 2;
		
		block_clear(&b);
		dir_op_offset = 0;
		log_block_read(&n, &log_block);
		add_dir_entry(&n, EXT2_FT_DIR, inode_id, name, &log_block);
		log_block_write(&n, &log_block);
		
		inode_write(parent_inode, &n);	
	}
	bitmap_write(bm_inodes, inode_id - 1, 1);

	
	

	
	inode_clear(&n);
	

	if(!fs_done)
	{
		n.uid = 0;
	} else {
		n.uid = current_ttyc()->uid;
	}

	n.mode = EXT2_S_IFDIR;
	n.size = 0;
	n.blocks = 0; // Beware! This represents the SECTORS!
	
	
	log_block = n.blocks / 2;
	
	block_clear(&b);
	dir_op_offset = 0;
	
	add_dir_entry(&n, EXT2_FT_DIR, inode_id, ".", &log_block);
	add_dir_entry(&n, EXT2_FT_DIR, parent_inode, "..", &log_block);

	log_block_write(&n, &log_block);
	
	n.blocks = (log_block) * 2;
	
	n.i_file_acl = 511;
	n._dir_inode = parent_inode;
	n._last_write_offset = dir_op_offset;
	
	inode_write(inode_id, &n);	
	



	
	fs_bitmaps_write_all();
	return inode_id;
}



static char pwd_string[1024];

char *	fs_pwd() {
	
	int j = 1023;
	int i = 0;
	
	for(; i < 1024; ++i) {
		pwd_string[i] = 0;
	}
		
	int start_inode = current_ttyc()->pwd;


	inode_read(start_inode, &n);


	while(n._dir_inode != start_inode) {
		int _old_dirnode = n._dir_inode;
		char * name = fs_iname(start_inode, n._dir_inode);
		int k = strlen(name) - 2;
		for(; k >= 0; k--, j--) {
			pwd_string[j] = name[k];
		}
		pwd_string[j] = '/';
		j--;
		start_inode = _old_dirnode;
		inode_read(start_inode, &n);
	}
	if(j == 1023)
	{
		pwd_string[j] = '/';
	} else {
		j++;
	}

	
	for(i = 0; j < 1024; i++,j++)
	{
		pwd_string[i] = pwd_string[j];
		pwd_string[j] = 0;
	}
		
	return pwd_string;
}

int fs_ls(char * data, int size, unsigned long * f_offset) {

	return fs_read_file(current_ttyc()->pwd, data, size, f_offset);
}

int	fs_cd(char * name) {
	int start_inode = current_ttyc()->pwd;
	
	int namenode = fs_indir(name, start_inode);
	if (namenode) {
		current_ttyc()->pwd = namenode;
		return 1;
	} else {
		return 0;
	}
}

unsigned int fs_has_perms(inode * n, int for_what) {
	if (!fs_done) {
		// If the kernel's not ready then we can do anything.
		// We're like, Chuck Norris, a superroot or Chuck Testa.
		return 1;
	}
	int current_uid = current_ttyc()->uid;
	int current_gid = user_gid(current_uid);
	int file_gid = user_gid(n->uid);
	int can = 0;

	if(for_what == ACTION_READ)
	{
		if (n->uid == current_uid && (n->i_file_acl & 0400)) {
			can = 1;
		}
		if (file_gid == current_gid && (n->i_file_acl & 0040)) {
			can = 1;
		}
		if (n->i_file_acl & 0004) {
			can = 1;
		}
	} else if(for_what == ACTION_WRITE)
	{
		if (n->uid == current_uid && (n->i_file_acl & 0200)) {
			can = 1;
		}
		if (file_gid == current_gid && (n->i_file_acl & 0020)) {
			can = 1;
		}
		if (n->i_file_acl & 0002) {
			can = 1;
		}
	}
	return can;
}

unsigned int fs_getown(char * filename) {
	int start_inode = current_ttyc()->pwd;
	int namenode = fs_indir(filename, start_inode);
	if (namenode) {
		inode_read(namenode, &n);
		return n.uid;
	} else {
		return -2;
	}
}

unsigned int fs_getmod(char * filename) {
	int start_inode = current_ttyc()->pwd;
	int namenode = fs_indir(filename, start_inode);
	if (namenode) {
		inode_read(namenode, &n);
		return n.i_file_acl & 0777;
	} else {
		return -2;
	}
}

unsigned int fs_chown(char * filename, char * username) {
	int new_uid = user_exists(username);
	int current_uid = current_ttyc()->uid;
	int start_inode = current_ttyc()->pwd;
	int namenode = fs_indir(filename, start_inode);

	if (namenode && new_uid != -1) {
		inode_read(namenode, &n);
		if (n.uid == current_uid || current_uid == 0) {
			n.uid = new_uid;
			inode_write(namenode, &n);
		} else {
			return -1;
		}
		return 1;
	} else {
		return -2;
	}
}

unsigned int fs_chmod(char * filename, int perms) {
	int current_uid = current_ttyc()->uid;
	int start_inode = current_ttyc()->pwd;
	int namenode = fs_indir(filename, start_inode);
	if (namenode) {
		inode_read(namenode, &n);
		if (n.uid == current_uid || current_uid == 0) {
			n.i_file_acl = perms;
			inode_write(namenode, &n);
		} else {
			return -1;
		}
		return 1;
	} else {
		return -2;
	}
}