#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"

#include "video.h"
#include "kernel.h"
#include "scheduler.h"


///////////// Inicio de Variables del Kernel

/* IDT de 80h entradas*/
DESCR_INT idt[0x81]; 
/* IDTR */
IDTR idtr; 

#define KERNEL_BUFFER_SIZE 16

int kernel_buffer[KERNEL_BUFFER_SIZE]; 

void clear_kernel_buffer() {
	int i = 0;
	for(i = 0; i < KERNEL_BUFFER_SIZE; ++i)	{
		kernel_buffer[i] = 0;
	}
}

///////////// Fin de Variables del Kernel

///////////// Inicio de funciones auxiliares del Kernel.

/*
 *	setup_IDT_entry
 * 		Inicializa un descriptor de la IDT
 *
 *	Recibe: Puntero a elemento de la IDT
 *	 Selector a cargar en el descriptor de interrupcion
 *	 Puntero a rutina de atencion de interrupcion
 *	 Derechos de acceso del segmento
 *	 Cero
 */
void setup_IDT_entry(DESCR_INT *item, byte selector, dword offset, byte access, byte cero) {
	item->selector = selector;
	item->offset_l = offset & 0xFFFF;
	item->offset_h = offset >> 16;
	item->access = access;
	item->cero = cero;
}

///////////// Fin de funciones auxiliares del kernel.

///////////// Inicio Handlers de interrupciones.


void int_09() {
	char scancode;
	scancode = _in(0x60);

	// We check if the scancode is a char or a control key.
	int flag = scancode >= 0x02 && scancode <= 0x0d;
	flag = flag || (scancode >= 0x10 && scancode <= 0x1b);
	flag = flag || (scancode >= 0x1E && scancode <= 0x29);
	flag = flag || (scancode >= 0x2b && scancode <= 0x35);
	if (flag)	{
		char sc = scanCodeToChar(scancode);
		if(sc != 0)	{
			pushC(sc); //guarda un char en el stack
		}
	}
	else {
		controlKey(scancode); // Envia el scancode al analizador de control keys.
	}

}




void int_80() {
	
	int systemCall = kernel_buffer[0];
	int fd         = kernel_buffer[1];
	int buffer     = kernel_buffer[2];
	int count      = kernel_buffer[3];

	
	int i, j;

	if (systemCall == WRITE) {
		Process * current = getp();
		fd_write(current->file_descriptors[fd],buffer,count);
	} else if (systemCall == READ) {
		Process * current = getp();
		fd_read(current->file_descriptors[fd],buffer,count);
	} else if (systemCall == OPEN) {
		fd_open((char *) systemCall, fd);
	} else if (systemCall == CLOSE) {
		fd_close((char *) systemCall, fd);
	}
}

///////////// Fin Handlers de interrupciones.


Process * p1, * idle, * kernel;



int idle_main(int argc, char ** params) {
	while(1) {
		_Halt();
	}
}


///////////// Inicio KMAIN

/**********************************************
 kmain()
 Punto de entrada de código C.
 *************************************************/
kmain() {
	int i, num;


	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE IRQ0    */

	setup_IDT_entry(&idt[0x08], 0x08, (dword) & _int_08_hand, ACS_INT, 0);

	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE IRQ1    */

	setup_IDT_entry(&idt[0x09], 0x08, (dword) & _int_09_hand, ACS_INT, 0);

	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE int80h    */

	setup_IDT_entry(&idt[0x80], 0x08, (dword) & _int_80_hand, ACS_INT, 0);

	/* Carga de IDTR */

	idtr.base = 0;
	idtr.base += (dword) & idt;
	idtr.limit = sizeof(idt) - 1;

	_lidt(&idtr);


	scheduler_init();
	_Cli();
	
	/* Habilito interrupcion de timer tick*/
	_mascaraPIC1(0xFC);
	_mascaraPIC2(0xFF);
	idle = create_process("idle", idle_main, 0, 0, 0, 0, 0, 0, 0, NULL);
	tty_init(0);
	tty_init(1);
	tty_init(2);
	tty_init(3);
	tty_init(4);
	tty_init(5);
	_Sti();


	// We soon exit out of here :)
	while (1);

}

///////////// Fin KMAIN