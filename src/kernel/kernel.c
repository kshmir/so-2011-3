#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"

#include "../startup/start.h"
#include "../drivers/keyboard.h"
#include "../drivers/video.h"
#include "../shell.h"

#include "scheduler.h"


///////////// Inicio de Variables del Kernel

/* IDT de 80h entradas*/
DESCR_INT idt[0x81]; 
/* IDTR */
IDTR idtr; 

// Counter of IRQ8 ticks since start.
int ticks = 0;
int cursor_ticks = 0;

// Counter of the video position
int videoPos = 0;

// Sets or not the cursor
int cursorEnabled = 1;
int hardCursorEnabled = 1;

// Direction to the video memory.
char *vidmem = (char *) 0xb8000;

// Stores the CPU frequency.
double cpuFreq = 0;

int fix_flag = 0;

///////////// Fin de Variables del Kernel

///////////// Inicio de funciones auxiliares del Kernel.

// TODO: Move me
int _ticks() {
	return cursor_ticks;
}

// TODO: Move me.
void setCursor(int b) {
	cursorEnabled = b;
	hardCursorEnabled = b;
}

// TODO: Move me.
void setVideoPos(int a) {
	videoPos = a;
	if (cursorEnabled)
		_setCursor(a / 2);
}

// This method can be cleaned up...
double* getFrequency(int precision, int tcks) {
	// Precision gives us the amount of times it'll be approximated.
	// Tcks gives us the amount of ticks to try to get the frecuency
	// 2 is the minimum value.
	int it = 0, n = precision, c = 0;
	unsigned long counter = 0;

	// We check limits...
	if (tcks > 18)
		tcks = 18;
	if (tcks < 2)
		tcks = 2;

	// Startup variables
	double cpuFreqs;
	cpuFreq = 0;

	// Iterate for elements.
	for (it = 0; it < n; it++) {
		ticks = 0;
		int oldticks = 1;
		// Waiting for a tick change helps us solve
		// Any redundancy in the numbers
		// "Syncing" the counter with the ticks is really helpful
		while (ticks != oldticks)
			;
		counter = _rdtsc();
		// Wait for another tick change, one is usually enough.
		while (ticks < tcks)
			;
		counter = _rdtsc() - counter;
		// Normalizes to Mhz
		cpuFreqs = counter / ((ticks - 1) * 54925.40115);
		cpuFreq += cpuFreqs;
		if (!fix_flag)
		{
			fix_flag++;
			cpuFreq = 0;
			it--;
		}
	}

	// Average if needed.
	cpuFreq /= n;
	return &cpuFreq;
}

// Escribre sobre s la cantidad n de data que se le manda desde c
void setBytes(void *s, char* c, int n) {
	unsigned char *p = s;
	int i;

	for (i = 0; i < n; i++) {
		p[i] = (char) c[i];
	}
}

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

void int_08() {
	ticks++;
	cursor_ticks++;
	if (hardCursorEnabled && cursor_ticks % 5 == 0) {
		cursorEnabled = !cursorEnabled;
		if (cursorEnabled)
			_setCursor(videoPos / 2);
		else
			_setCursor(-1);
	}

}

void int_09() {
	char scancode;
	char eoi = EOI;
	_read(KEYBOARD, &scancode, 1);

	// We check if the scancode is a char or a control key.
	int flag = scancode >= 0x02 && scancode <= 0x0d;
	flag = flag || (scancode >= 0x10 && scancode <= 0x1b);
	flag = flag || (scancode >= 0x1E && scancode <= 0x29);
	flag = flag || (scancode >= 0x2b && scancode <= 0x35);
	if (flag) {
		pushC(scanCodeToChar(scancode)); //guarda un char en el stack
	}
	else {
		controlKey(scancode); // Envia el scancode al analizador de control keys.
	}

	_write(PIC1, &eoi, 1);
}

void int_80(int systemCall, int fd, char *buffer, int count) {
	int i, j;

	if (systemCall == WRITE) //write
	{
		if (fd == STDOUT) //PANTALL
		{
			setBytes(vidmem + videoPos, buffer, count);
		} else if (fd == PIC1) {
			_out(0x20, buffer[0]);
		}
	} else if (systemCall == READ) //read
	{
		if (fd == KEYBOARD) {
			buffer[0] = _in(0x60);
		}
	}
}

///////////// Fin Handlers de interrupciones.


Process * p1, * idle, * kernel;

int thinked3 = 0;

int p1_main(int argc, char ** params) {
	int var = 0;
	while(1) {
		if(var < 10240) {
			var++;
		}
		printf("I am the main... %d %d\n", var, thinked3);
		_yield();
	}	
}

int kernel_main(int argc, char ** params) {
	while(1) {
		if(thinked3 < 102400) {
			thinked3++;
		}
		_yield();
	}
}

int idle_main(int argc, char ** params) {
	printf("Idle Start...");		 
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

	startKeyboard();
	initVideo();


	scheduler_init();
	_Cli();
	
	/* Habilito interrupcion de timer tick*/
	_mascaraPIC1(0xFC);
	_mascaraPIC2(0xFF);
	

	idle = create_process("idle", idle_main, 0, 1);
	tty_init();
	_Sti();


	// We soon exit out of here :)
	while (1);

}

///////////// Fin KMAIN