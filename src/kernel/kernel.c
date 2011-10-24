#include "../../include/kernel.h"
#include "../../include/kasm.h"
#include "../../include/defs.h"


#include "video.h"
#include "kernel.h"
#include "tty.h"
#include "scheduler.h"
#include "fd.h"


///////////// Inicio de Variables del Kernel

/* IDT de 80h entradas*/
DESCR_INT idt[0x81]; 
/* IDTR */
IDTR idtr; 



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

int krn = 0;

int ready = 0;

void setready() {
	ready = 1;
}

int kernel_rd() {
	return ready;
}

int kernel_ready() {
 	_in(0x60);	 // Needed for not blcoking the keyboard on idleness
	return ready;
}

void int_09() {
	krn++;
	char scancode;
	scancode = _in(0x60);

	// We check if the scancode is a char or a control key.
	int flag = scancode >= 0x02 && scancode <= 0x0d;
	flag = flag || (scancode >= 0x10 && scancode <= 0x1b);
	flag = flag || (scancode >= 0x1E && scancode <= 0x29);
	flag = flag || (scancode >= 0x2b && scancode <= 0x35);
	if (flag)	{
		char sc = scanCodeToChar(scancode);
		if(sc != 0 && sc != EOF)	{
			pushC(sc); //guarda un char en el stack
		}
	}
	else {
		controlKey(scancode); // Envia el scancode al analizador de control keys.
	}
	
	kernel_buffer[0] = KILL;
	
	krn--;
	
}

int in_kernel(){
	return krn;
}

void int_80() {
	if(krn)
	{
		return;
	}
	
	krn++;
	int systemCall = kernel_buffer[0];
	int fd         = kernel_buffer[1];
	int buffer     = kernel_buffer[2];
	int count      = kernel_buffer[3];

	
	int i, j;
	Process * current;
	Process * p;
	int inode;
	int _fd;
	// Yeah, wanna know why we don't access an array directly? ... Because of big bugs we had that way.
	switch(systemCall) {
		case READY:
			kernel_buffer[KERNEL_RETURN] = kernel_ready();
			break;
		case WRITE:
 			current = getp();
			kernel_buffer[KERNEL_RETURN] = fd_write(current->file_descriptors[fd],(char *)buffer,count);
			break;
		case READ:
			current = getp();
			kernel_buffer[KERNEL_RETURN] = fd_read(current->file_descriptors[fd],(char *)buffer,count);
			break;
		case MKFIFO:
			_fd = process_getfreefd();
 			fd = fd_open(_FD_FIFO, (void *)kernel_buffer[1],kernel_buffer[2]);
			if(_fd != -1 && fd != -1)	{
				getp()->file_descriptors[_fd] = fd;
				kernel_buffer[KERNEL_RETURN] = _fd;
			}
			else {
				kernel_buffer[KERNEL_RETURN] = -1;
			}
			break;
		case OPEN:
			_fd = process_getfreefd();
			fd = fd_open(_FD_FILE, (void *) kernel_buffer[1], kernel_buffer[2]);
			if(_fd != -1)
			{

				getp()->file_descriptors[_fd] = fd;
				kernel_buffer[KERNEL_RETURN] = _fd;
			}
			else {
				kernel_buffer[KERNEL_RETURN] = -1;
			}
			break;
		case CLOSE:
			kernel_buffer[KERNEL_RETURN] = fd_close(getp()->file_descriptors[fd]);
			break;
		case PCREATE:
			kernel_buffer[KERNEL_RETURN] = sched_pcreate(kernel_buffer[1],kernel_buffer[2],kernel_buffer[3]);
			break;
		case PRUN:
			kernel_buffer[KERNEL_RETURN] = sched_prun(kernel_buffer[1]);
			break;
		case PDUP2:
			kernel_buffer[KERNEL_RETURN] = sched_pdup2(kernel_buffer[1],kernel_buffer[2],kernel_buffer[3]);
			break;
		case GETPID:
			kernel_buffer[KERNEL_RETURN] = sched_getpid();
			break;
		case WAITPID:
			kernel_buffer[KERNEL_RETURN] = sched_waitpid(kernel_buffer[1]);
			break;
		case PTICKS:
			kernel_buffer[KERNEL_RETURN] = (int) storage_index();
			break;
		case PNAME:
			p = process_getbypid(kernel_buffer[1]);
			if(p == NULL)
			{
				kernel_buffer[KERNEL_RETURN] = (int) NULL;
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) p->name;
			}
			break;
		case PSTATUS:
			p = process_getbypid(kernel_buffer[1]);
			if(p == NULL)
			{
				kernel_buffer[KERNEL_RETURN] = (int) -1;
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) p->state;
			}
			break;
		case PPRIORITY:
			p = process_getbypid(kernel_buffer[1]);
			if(p == NULL)
			{
				kernel_buffer[KERNEL_RETURN] = (int) -1;
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) p->priority;
			}
			break;
		case PGID:
			p = process_getbypid(kernel_buffer[1]);
			if(p == NULL)
			{
				kernel_buffer[KERNEL_RETURN] = (int) -1;
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) p->gid;
			}
			break;
		case PGETPID_AT:
			p = process_getbypindex(kernel_buffer[1]);
			if (p->state != -1) {
				kernel_buffer[KERNEL_RETURN] = (int) p->pid;
			} else {
				kernel_buffer[KERNEL_RETURN] = -1;
			}
			break;
		case KILL:
			kernel_buffer[KERNEL_RETURN - 1] = kernel_buffer[1];
			kernel_buffer[KERNEL_RETURN - 2] = kernel_buffer[2];
			break;
		case PSETP:
			p = process_getbypid(kernel_buffer[1]);
			if(p == NULL)	{
				kernel_buffer[KERNEL_RETURN] = (int) -1;
			} else {
				if(kernel_buffer[2] <= 4 && kernel_buffer[2] >= 0)	{
					p->priority = kernel_buffer[2];
				}
				kernel_buffer[KERNEL_RETURN] = (int) p->gid;
			}
			break;
		case SETSCHED:
			sched_set_mode(kernel_buffer[1]);
			break;
		case PWD:
			kernel_buffer[KERNEL_RETURN] = (int) fs_pwd();
			break;
		case CD:
			kernel_buffer[KERNEL_RETURN] = (int) fs_cd(kernel_buffer[1]);
			break;
		case MOUNT:
			fs_init();
			break;
		case LS:
			kernel_buffer[KERNEL_RETURN] = (int) fs_ls(kernel_buffer[1],kernel_buffer[2],kernel_buffer[3]);
			break;
		case MKDIR:
			kernel_buffer[KERNEL_RETURN] = (int) fs_mkdir(kernel_buffer[1],current_ttyc()->pwd);
			break;
		case RM:
			inode = fs_indir(kernel_buffer[1],current_ttyc()->pwd);
			if (inode) {
				kernel_buffer[KERNEL_RETURN] = (int) fs_rm(inode,0);
			}
			break;
		case GETUID:
			if(kernel_buffer[1] == 0)
			{
				kernel_buffer[KERNEL_RETURN] = (int) current_ttyc()->uid;
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) user_exists(kernel_buffer[1]);
			}
			break;
		case GETGID:
			if(kernel_buffer[1] == 0)
			{
				kernel_buffer[KERNEL_RETURN] = (int) user_gid(current_ttyc()->uid);
			} else {
				kernel_buffer[KERNEL_RETURN] = (int) user_gid(kernel_buffer[1]);
			}

			break;
		case MAKEUSER:
			kernel_buffer[KERNEL_RETURN] = user_create(kernel_buffer[1],
							kernel_buffer[2], user_gid(current_ttyc()->uid));
			break;
		case SETGID:
			kernel_buffer[KERNEL_RETURN] = user_setgid(kernel_buffer[1], 
													   kernel_buffer[2]);
			break;
		case UDELETE:
			kernel_buffer[KERNEL_RETURN] = user_delete(kernel_buffer[1]);
			break;
		case UEXISTS:
			kernel_buffer[KERNEL_RETURN] = user_exists(kernel_buffer[1]);
			break;
		case ULOGIN:
			kernel_buffer[KERNEL_RETURN] = user_login(kernel_buffer[1], 
													  kernel_buffer[2]);
			break;
		case ULOGOUT:
			kernel_buffer[KERNEL_RETURN] = user_logout();
			break;
		case CHOWN:
			kernel_buffer[KERNEL_RETURN] = fs_chown(kernel_buffer[1],
													kernel_buffer[2]);
			break;
		case CHMOD:
			kernel_buffer[KERNEL_RETURN] = fs_chmod(kernel_buffer[1],
												    kernel_buffer[2]);
			break;
		case GETOWN:
			kernel_buffer[KERNEL_RETURN] = fs_getown(kernel_buffer[1]);
			break;
		case GETMOD:
			kernel_buffer[KERNEL_RETURN] = fs_getmod(kernel_buffer[1]);
			break;
		default:
			break;
	}
	
	krn--;
}


// Fires a signal after a syscall, only if the kernel has been set to do so.
void signal_on_demand() {

	if (kernel_buffer[KERNEL_RETURN - 1] != 0) {
		
		if (kernel_buffer[0] == KILL) {
			make_atomic();
			int sigcode = kernel_buffer[KERNEL_RETURN - 1];
			int pid = kernel_buffer[KERNEL_RETURN - 2];

			kernel_buffer[KERNEL_RETURN - 1] = 0; // SIGCODE
			kernel_buffer[KERNEL_RETURN - 2] = 0; // PID
		
			sg_handle(sigcode, pid);
			release_atomic();
		}
		kernel_buffer[KERNEL_RETURN - 1] = 0; // SIGCODE
		kernel_buffer[KERNEL_RETURN - 2] = 0; // PID
	}

}

///////////// Fin Handlers de interrupciones.

Process * p1, * idle, * kernel;

int idle_main(int argc, char ** params) {
	

	int i = 0;
	char a[] = { ' ', 0x07 };
	
	for(; i < 2000; ++i)	{
		memcpy((char*)0xb8000 + i * 2, a, 2);
	}
	
	
	// The funny message, windows says starting windows... why shouldn't we? D:
	char start_msg[][2] ={ 
		{ 'S', 0x07 },
		{ 't', 0x08 },
		{ 'a', 0x09 },
		{ 'r', 0x0a },
		{ 't', 0x0b },
		{ 'i', 0x0c },
		{ 'n', 0x0d },
		{ 'g', 0x0e },
		{ ' ', 0x0f },
		{ 'M', 0x02 },
		{ 'o', 0x03 },
		{ 'n', 0x04 },
		{ 'i', 0x05 },
		{ 'x', 0x06 },
		{ ' ', 0x0f },
		{ '\001', 0x5f },
	};
	
	i = 0;
	for(; i < 16; ++i)
	{
		if(i == 15)
		{
			_Halt();_Halt();_Halt();_Halt();
		}
		memcpy((char*)0xb8000 + i * 2, start_msg[i], 2);
		_setCursor(i);
	}
	make_atomic();
	mount();
	users_init();
	tty_init(0);
	tty_init(1);
	tty_init(2);
	tty_init(3);
	tty_init(4);
	tty_init(5);

	release_atomic();
	
	setready(); // Now we can read the keyboard
	
	while(1) {
		fs_finish();		
		_Halt(); // Now set to idle.
	}
}

void disk()	{
	printf("Wo\n");
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
	
	/* CARGA DE IDT CON LA RUTINA DE ATENCION DE int79h    */

	setup_IDT_entry(&idt[0x79], 0x08, (dword) & _int_79_hand, ACS_INT, 0);

	/* Carga de IDTR */

	idtr.base = 0;
	idtr.base += (dword) & idt;
	idtr.limit = sizeof(idt) - 1;

	_lidt(&idtr);



	scheduler_init();
	
	Cli();

	/* Habilito interrupcion de timer tick*/
	_mascaraPIC1(0xFC);
	_mascaraPIC2(0xFF);

	Sti();





	idle = create_process("idle", idle_main, 0, 0, 0, 0, 0, 0, 0, NULL, 0);
	
	

	// We soon exit out of here :)
	while (1);

}

///////////// Fin KMAIN