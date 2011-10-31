GLOBAL  _read_msw,_lidt
GLOBAL  _int_08_hand
GLOBAL  _int_09_hand
GLOBAL	_int_80_hand
GLOBAL	_int_79_hand
GLOBAL  _mascaraPIC1,_mascaraPIC2,_Cli,_Sti
GLOBAL  _debug
GLOBAL	_setCursor
GLOBAL	_restart
GLOBAL	_Cli
GLOBAL	_Sti
GLOBAL	_in
GLOBAL	_out
GLOBAL	_inw
GLOBAL	_outw
GLOBAL  __stack_chk_fail
GLOBAL	_rdtsc

GLOBAL _GetCS
GLOBAL _GetESP
GLOBAL _Halt
GLOBAL _yield

; Syscalls
GLOBAL	read
GLOBAL	write
GLOBAL	open
GLOBAL	mkfifo
GLOBAL	close
GLOBAL	pcreate
GLOBAL	prun
GLOBAL	pdup2
GLOBAL	getpid
GLOBAL	waitpid
GLOBAL	pticks
GLOBAL	pname
GLOBAL	ppriority
GLOBAL	pstatus
GLOBAL	pgid
GLOBAL	pgetpid_at
GLOBAL	kill
GLOBAL	psetp
GLOBAL	setsched
GLOBAL	pwd
GLOBAL	cd
GLOBAL	ls
GLOBAL	mount
GLOBAL	mkdir
GLOBAL	rm
GLOBAL	getuid
GLOBAL	getgid
GLOBAL	makeuser
GLOBAL	setgid
GLOBAL	udelete
GLOBAL	uexists
GLOBAL	ulogin
GLOBAL	chown
GLOBAL	chmod
GLOBAL	logout
GLOBAL	fgetown
GLOBAL	fgetmod
GLOBAL	makelink
GLOBAL	cp
GLOBAL	mv
GLOBAL	fsstat
GLOBAL	sleep

GLOBAL	_rtc
GLOBAL	_inb
GLOBAL	_outb

EXTERN	scheduler_tick
EXTERN	signal_on_demand
EXTERN  int_08
EXTERN  int_09
EXTERN	int_80
EXTERN	scheduler_save_esp
EXTERN	kernel_buffer
EXTERN	scheduler_get_temp_esp
EXTERN	scheduler_think
EXTERN	scheduler_load_esp
EXTERN	softyield
EXTERN	Cli
EXTERN	kernel_ready
EXTERN	Sti




SECTION .text

_Cli:
		cli			; limpia flag de interrupciones
		ret

_Sti:
		sti			; habilita interrupciones por flag
		ret
		

_Halt:			; Should lock everything?
		hlt			; wait for HPET/PIT
		ret
		
_GetCS: 		; For debugging
		mov eax, cs
		ret
		
_GetESP:		; For debugging
		mov eax, esp
		ret

_mascaraPIC1:			; Escribe mascara del PIC 1
		push	ebp
		mov		ebp, esp
		mov		ax, [ss:ebp+8]  ; ax = mascara de 16 bits
		out		21h,al
		pop		ebp
		ret

_mascaraPIC2:			; Escribe mascara del PIC 2
		push	ebp
		mov		ebp, esp
		mov		ax, [ss:ebp+8]  ; ax = mascara de 16 bits
		out		0A1h,al
		pop		ebp
		ret

_read_msw:
        smsw    ax		; Obtiene la Machine Status Word
        retn

; Carga el IDTR
_lidt:
		push	ebp
		mov		ebp, esp
		push	ebx
		mov		ebx, [ss: ebp + 6] ; ds:bx = puntero a IDTR
		rol		ebx,16
		lidt	[ds: ebx]          ; carga IDTR
		pop		ebx
		pop		ebp
		retn

_yield:
		int 08h
		ret

; Handler de INT 8 (Timer tick)
_int_08_hand:
		call Cli
		pushad
			mov eax, esp
			push eax
				call scheduler_save_esp
			pop eax
			
			call scheduler_get_temp_esp
			mov esp, eax
			
			call scheduler_think
			
			call scheduler_load_esp
			mov esp,eax
			;call _debug;
		popad
		mov al,20h			; Envio de EOI generico al PIC
		out 20h,al
		call Sti
		iret

__stack_chk_fail:
		ret


_rtc:
		call Cli
		call scheduler_tick
		mov 	al,020h			; Envio de EOI generico al PIC
		out 	020h,al
		mov 	al,0a0h			; Envio de EOI generico al PIC
		out 	0a0h,al
		call Sti
		iret
		
		
_setCursor:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		bx, [ebp+8]  	; lo que se envia
		mov		al,0x0e
		mov		dx,0x03d4
		out		dx, al
		mov		al,bh
		mov		dx,0x03d5
		out		dx, al
		mov		al,0x0f
		mov		dx,0x03d4
		out		dx, al
		mov		al,bl
		mov		dx,0x03d5
		out		dx, al
		pop		ebp
		ret

_restart:
		mov		al,0xfe
		out		0x64,al
		ret

_inb:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in byte		al, dx
		pop		ebp
		ret

_outb:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out 	dx, al
		pop		ebp
		ret

_in:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in		al, dx
		pop		ebp
		ret

_out:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out		dx, al
		pop		ebp
		ret

_inw:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in		ax, dx
		pop		ebp
		ret

_outw:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out		dx, ax
		pop		ebp
		ret


_rdtsc:
		push	ebp
		mov		ebp, esp		; Stack frame
		rdtsc
		mov		esp,ebp
		pop		ebp
		ret

; Debug para el BOCHS, detiene la ejecución Para continuar colocar en el BOCHSDBG: set $eax=0



_debug:
		push	bp
		mov		bp, sp
		push	ax
vuelve:	
		mov		ax, 1
		cmp		ax, 0
		jne		vuelve
		pop		ax
		pop		bp
		retn
		
		

_fast_exit:
		mov 	al,20h			; Envio de EOI generico al PIC
		out 	20h,al
		call Sti
		
		iret

_int_09_hand:
		call kernel_ready
		cmp eax, 0
		jne _09hand
		call Cli
		mov al,20h			; Envio de EOI generico al PIC
		out 20h,al
		call Sti
		iret
	_09hand:
		call Cli
		pushad
			mov eax, esp
			push eax
				call scheduler_save_esp
			pop eax
	
			call scheduler_get_temp_esp
			mov esp, eax
	
			call int_09
	
			call scheduler_load_esp
			mov esp,eax
			;call _debug;
		popad
		mov 	al,20h			; Envio de EOI generico al PIC
		out 	20h,al
		call Sti
		
		int		079h
		
		iret

; recibe parametros a traves de los registros
; aex -> 0 para write, 1 para read
; ebx -> file descriptor
; ecx -> direccion de la cadena a escribir
; edx -> cantidad de caracteres a escribir

_int_80_hand:
		call Cli
		push	ds
		push	es
		pushad										; We push everything
		
		mov 	[kernel_buffer + 12], edx			; Write to the kernel.
		mov 	[kernel_buffer + 8],  ecx
		mov 	[kernel_buffer + 4],  ebx
		mov 	[kernel_buffer],      eax
		
		; TODO: Put back the stack exchange here, broken god knows why by the disk driver.
		
		call int_80									; Make the syscall
		
		popad
		pop		es
		pop		ds
		call Sti
		
		int		079h ; Signals
		
		iret
		
_int_79_hand:
		push	ds
		push	es
		pushad														; We push everything

		call signal_on_demand									; Make the syscall

		popad
		pop		es
		pop		ds
		
		iret

read:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 3				; eax en 3 para read
		mov 	ebx, [ebp+8]		; file descriptor
		mov 	ecx, [ebp+12]		; buffer donde escribir
		mov 	edx, [ebp+16]		; cantidad
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		
		mov		eax, [kernel_buffer + 60]
		cmp		eax, -2
		jne		_read_jmp_0
		call	softyield
		jmp		read
	_read_jmp_0:
	
		ret
		
write:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov		eax, 4				; eax en 4 para write
		mov 	ebx, [ebp+8]		; file descriptor
		mov 	ecx, [ebp+12]		; buffer a escribiar
		mov 	edx, [ebp+16]		; cantidad
		int 	80h
		popa
		mov 	esp,ebp
		pop 	ebp

		mov		eax, [kernel_buffer + 60]

		cmp		eax, -2
		jne		_write_jmp_0
		call	softyield
		jmp		write
	_write_jmp_0:

		ret

open:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 5				; eax en 5 para open
		mov 	ebx, [ebp+8]		; file names
		mov 	ecx, [ebp+12]		; permissions
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
close:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 6				; eax en 6 para close
		mov 	ebx, [ebp+8]		; file descriptor
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]			
		ret

mkfifo:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 7				; eax en 7 para mkfifo
		mov 	ebx, [ebp+8]		; fifo name
		mov 	ecx, [ebp+12]		; perms
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pcreate:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 8				; eax en 8 para pcreate
		mov 	ebx, [ebp+8]		; file descriptor
		mov 	ecx, [ebp+12]		; buffer donde escribir
		mov 	edx, [ebp+16]		; cantidad
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
prun:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 9				; eax en 9 para prun
		mov 	ebx, [ebp+8]		; pid
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pdup2:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 10				; eax en 10 para pdup2
		mov 	ebx, [ebp+8]		; file descriptor
		mov 	ecx, [ebp+12]		; file descriptor
		mov 	edx, [ebp+16]		; file descriptor
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
getpid:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 11				; eax en 11 para getpid
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		ret

waitpid:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov 	eax, 12				; eax en 9 para prun
		mov 	ebx, [ebp+8]		; pid
		int 	80h
		popa
		mov 	esp, ebp
		pop 	ebp
		mov		eax, [kernel_buffer + 60]
		
		cmp		eax, -1	
		je		_waitpid_jmp_0
		call	_yield
	_waitpid_jmp_0:
		ret

pticks:
		push 	ebp
		mov 	ebp, esp
		pusha
		mov		eax, 13				; eax en 13 para pticks
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret

pname:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 14				; eax en 14 para pname
		mov 	ebx, [ebp+8]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pstatus:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 15				; eax en 15 para pstatus
		mov 	ebx, [ebp+8]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
ppriority:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 16				; eax en 16 para ppriority
		mov 	ebx, [ebp+8]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pgid:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 17				; eax en 17 para pgid
		mov 	ebx, [ebp+8]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pgetpid_at:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 18				; eax en 18 para pgetpid_at
		mov 	ebx, [ebp+8]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
kill:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 19				; eax en 19 para kill
		mov 	ebx, [ebp+8]		; signal
		mov 	ecx, [ebp+12]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
psetp:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 20				; eax en 20 para psetp
		mov 	ebx, [ebp+8]		; pid
		mov 	ecx, [ebp+12]		; priority
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
setsched:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 21				; eax en 21 para setsched
		mov 	ebx, [ebp+8]		; schedmode
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
pwd:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 22				; eax en 22 para pwd
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
cd:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 23				; eax en 23 para cd
		mov 	ebx, [ebp+8]		; name of file to move
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret

ls:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 24				; eax en 24 para ls
		mov 	ebx, [ebp+8]		; file descriptor
		mov 	ecx, [ebp+12]		; file descriptor
		mov 	edx, [ebp+16]		; file descriptor
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret

mount:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 25				; eax en 25 para mount
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
mkdir:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 26				; eax en 25 para pwd
		mov 	ebx, [ebp+8]		; file file name
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
rm:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 27				; eax en 27 para rm
		mov 	ebx, [ebp+8]		; file name
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
getuid:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 28				; eax en 28 para ls
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
getgid:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 29				; eax en 29 para getgid
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
makeuser:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 30				; eax en 30 para makeuser
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
setgid:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 31				; eax en 31 para setgid
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
udelete:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 32				; eax en 32 para udelete
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
uexists:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 33				; eax en 33 para uexists
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
ulogin:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 34				; eax en 34 para ulogin
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
chown:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 35				; eax en 35 para chown
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
chmod:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 36				; eax en 36 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
logout:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 37				; eax en 37 para logout
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
fgetown:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 38				; eax en 38 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
fgetmod:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 39				; eax en 39 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
cp:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 40				; eax en 40 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
mv:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 41				; eax en 41 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
makelink:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 42				; eax en 42 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret

fsstat:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 43				; eax en 43 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret
		
sleep:
		push	ebp
		mov		ebp, esp
		pusha
		mov		eax, 44				; eax en 43 para chmod
		mov 	ebx, [ebp+8]		
		mov 	ecx, [ebp+12]		
		mov 	edx, [ebp+16]		
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		call	softyield
		ret