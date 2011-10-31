/*********************************************
 kasm.h
 ************************************************/

#include "defs.h"


// Sets the IDTR register
void _lidt(IDTR *idtr);

// Restarts the PC
void _restart();

// Sets the cursor
void _setCursor(int a);

/* Writes PIC1's mask */
void _mascaraPIC1(byte mascara);
/* Writes PIC2's mask */
void _mascaraPIC2(byte mascara);

/* Disables interrupts */
void _Cli(void);
/* Enables interrutps  */
void _Sti(void);

/* Timer tick */
void _int_08_hand();
/* Keyboard */
void _int_09_hand();
/* Handler INT 80h */
void _int_80_hand(int systemCall, int fd, char *buffer, int count);
// For signals
void _int_79_hand();

// RDTSC
int _rdtsc();

/* System call write */
void _write ( int fd, char *buffer, int count );

/* System call read */
void _read ( int fd, void *buffer, size_t count );

/* Assembly's out */
int _out(unsigned int port, int value);

/* Assembly's in */
int _in(unsigned int port);

/* Call for debug */
void _debug(void);