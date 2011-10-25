#include "atadisk.h"

#define BIT0 1
#define BIT1 1<<1
#define BIT2 1<<2
#define BIT3 1<<3
#define BIT4 1<<4
#define BIT5 1<<5
#define BIT6 1<<6
#define BIT7 1<<7
#define BIT8 1<<8
#define BIT9 1<<9
#define BIT10 1<<10
#define BIT11 1<<11
#define BIT12 1<<12
#define BIT13 1<<13
#define BIT14 1<<14
#define BIT15 1<<15

/* This defines are to check the sepecifications of the disk.
	#TODO: check size!*/
#define IS_REMOVABLE(D) (D & BIT7) ? printf("Is removable\n") : printf("Is not removable\n")
#define IS_ATA_DRIVE(D) (D & BIT15) ? printf("Is not ATA\n") : printf("Is ATA\n")

#define DMA_SUP(D) (D & BIT8) ? printf("DMA is supported\n") : printf("DMA is not supported\n")
#define LBA_SUP(D) (D & BIT9) ? printf("LBA is supported\n") : printf("LBA is not supported\n")
#define DMA_QUEUED_SUP(D) (D & BIT1) ? printf("DMA QUEUED supported\n") : printf("DMA QUEUED is not supported\n")

#define SECTOR_SIZE 512


// To read N bytes from hard disk, must alloc N+1 bytes for ans, as N+1 byte is used to null-character
void _disk_read(int ata, char * ans, int numreads, unsigned int sector){
	
	// We need this to make it work, I just don't know why

	Sti();
	_Halt();
	_outw(0x3F6, 0);

	int i = 0;
	for(i = 0; i < SECTOR_SIZE; ++i)
	{
		ans[i] = 0;
	}
	
	
	unsigned long long addr = ((unsigned long long)sector) & 0x00000000ffffffff;
	// printf("error: %d\n", getErrorRegister(ATA0));
	
	_outb(0x1F1, 0x00); 
	_outb(0x1F1, 0x00);
	_outb(0x1F2, 0x00); 
	_outb(0x1F2, numreads);
	_outb(0x1F3, (unsigned char)(addr >> 24));
	_outb(0x1F3, (unsigned char)addr);
	_outb(0x1F4, (unsigned char)(addr >> 32));
	_outb(0x1F4, (unsigned char)(addr >> 8));
	_outb(0x1F5, (unsigned char)(addr >> 40));
	_outb(0x1F5, (unsigned char)(addr >> 16));
	_outb(0x1F6, 0);
	_outb(0x1F7, 0x24);
	
	

	
	// while ((_inw(0x3F6) & (BIT7)));
	// printf("error: %d\n", getErrorRegister(ATA0));		
	int b;
	unsigned short data;
	int errors = 0;
	int c = 0;
	for(i = 0; i < numreads; ++i)
	{
		while (!(_inb(0x1F7) & 0x08)) {}
		for(b=0 ; b < SECTOR_SIZE ; b+=2, c+=2){
			data = getDataRegister(ata);
			translateBytes(ans+c, data, sector);
		}
	}
	// printf("error: %d\n", getErrorRegister(ATA0));
	_outw(0x3F6, 0);
	Cli();
}

// Translate one word into two char
void translateBytes(char * ans, unsigned short databyte, int sector){	
	ans[0] = databyte & 0xFF;
	ans[1] = databyte >> 8;
}


// Writes to the ata chosen the msg received the ammount of bytes requested starting from the secto chose + the offset
void _disk_write(int ata, char * msg, int numreads, unsigned int sector){
	

	Sti();
	_Halt();
	_outw(0x3F6, 0);


	ata=ATA0;
	int i = 0;

	unsigned long long addr = ((unsigned long long)sector) & 0x00000000ffffffff;

	_outb(0x1F1, 0x00); 
	_outb(0x1F1, 0x00);
	_outb(0x1F2, 0x00); 
	_outb(0x1F2, numreads);
	_outb(0x1F3, (unsigned char)(addr >> 24));
	_outb(0x1F3, (unsigned char)addr);
	_outb(0x1F4, (unsigned char)(addr >> 32));
	_outb(0x1F4, (unsigned char)(addr >> 8));
	_outb(0x1F5, (unsigned char)(addr >> 40));
	_outb(0x1F5, (unsigned char)(addr >> 16));
	_outb(0x1F6, 0);
	_outb(0x1F7, 0x34);

	while ((_inw(0x1F7) & (BIT7)));


	// Now write all the sector
	int b;
	int c = 0;
	for(i = 0; i < numreads; ++i)
	{
		while (!(_inb(0x1F7) & 0x08)) {}
		for (b=0; b<SECTOR_SIZE; b+=2, c+=2) {
			writeDataToRegister(ata, msg[c+1], msg[c]);
		}
	}

	_outw(0x3F6, 0);
	Cli();

}

void writeDataToRegister(int ata, char upper, char lower){
	
	unsigned short out;
	
	// Wait for driver's ready signal.
	while (!(_inw(0x1F7) & (BIT3)));
	
	out = (upper << 8) | lower;
	_outw(ata + WIN_REG0, out);
	
}

unsigned short getDataRegister(int ata){

	unsigned short ans;

	while (!(_inw(0x1F7) & (BIT3)));
	ans = _inw(ata + WIN_REG0);

	return ans;
}

unsigned short getErrorRegister(int ata){

	unsigned short rta = _in(ata + WIN_REG1) & 0x00000FFFF;

	return rta;
}

// Send a command to the disk in order to read or write
void sendComm(int ata, int rdwr, unsigned int sector){

	while (!(_inw(0x3F6) & (BIT6)));
	
	_out(ata + WIN_REG1, 0);
	_out(ata + WIN_REG2, 1);	// Set count register sector in 1
	
	_out(ata + WIN_REG3, (unsigned char)sector);			// LBA low
	_out(ata + WIN_REG4, (unsigned char)(sector >> 8));		// LBA mid
	_out(ata + WIN_REG5, (unsigned char)(sector >> 16));	// LBA high
	_out(ata + WIN_REG6, 0);
	
	// Set command
	_out(ata + WIN_REG7, rdwr);
}

unsigned short getStatusRegister(int ata){
	unsigned short rta;
	_Cli();
	rta = _in(ata + WIN_REG7) & 0x00000FFFF;
	_Sti();
	return rta;
}

void identifyDevice(int ata){

	_out(ata + WIN_REG6, 0xA0);
	_out(ata + WIN_REG7, WIN_IDENTIFY);

}

// Check disk features
void check_drive(int ata){
	
    identifyDevice(ata);


	unsigned short data = 0;

	char msg[512];
	unsigned short sector=12;
	int offset=0;
	int count=512;
	_Halt();

    int i;
    for(i=0;i<255;i++){
        data = getDataRegister(ata);
		
		switch(i){
			case 47:
			case 59:
			printf("%d\n", data);
		}
    }
}

