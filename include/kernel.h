/********************************** 
 *
 *  Kernel
 *
 ***********************************/
#ifndef _kernel_
#define _kernel_

#include "defs.h"

#define OS_PID	0

int _ticks();

void setCursor(int b);

void setup_IDT_entry(DESCR_INT * item, byte selector, dword offset, byte access, byte cero);

/* __write
 * Recibe como parametros:
 * - File Descriptor
 * - Buffer del source
 * - Cantidad
 **/
size_t __write(int fd, const void* buffer, size_t count);

/* __read
 * Recibe como parametros:
 * - File Descriptor
 * - Buffer a donde escribir
 * - Cantidad
 **/
size_t __read(int fd, void* buffer, size_t count);

#endif
