GLOBAL  _read_msw,_lidt
GLOBAL  _int_08_hand
GLOBAL  _int_09_hand
GLOBAL	_int_80_hand
GLOBAL	_int_79_hand
GLOBAL  _mascaraPIC1,_mascaraPIC2,_Cli,_Sti
GLOBAL  _debug
GLOBAL	_setCursor
GLOBAL	_restart
GLOBAL	_in
GLOBAL	_out
GLOBAL  __stack_chk_fail
GLOBAL	_rdtsc

GLOBAL _GetCS
GLOBAL _GetESP
GLOBAL _Halt
GLOBAL _yield

; Syscalls
GLOBAL	read
GLOBAL	write
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



SECTION .text

_Cli:
		cli			; limpia flag de interrupciones
		ret

_Sti:
		sti			; habilita interrupciones por flag
		ret
		
; TODO: Improve this
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
		retn

_mascaraPIC2:			; Escribe mascara del PIC 2
		push	ebp
		mov		ebp, esp
		mov		ax, [ss:ebp+8]  ; ax = mascara de 16 bits
		out		0A1h,al
		pop		ebp
		retn

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

; Handler de INT 8 ( Timer tick)
_int_08_hand:
		cli
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
		sti
		iret

__stack_chk_fail:
		ret


_int_09_hand:
		cli
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
		sti	
		
		int		079h
		
		iret

; recibe parametros a traves de los registros
; aex -> 0 para write, 1 para read
; ebx -> file descriptor
; ecx -> direccion de la cadena a escribir
; edx -> cantidad de caracteres a escribir

_int_80_hand:
		cli
		push	ds
		push	es
		pushad										; We push everything
		
		mov 	[kernel_buffer + 12], edx			; Write to the kernel.
		mov 	[kernel_buffer + 8],  ecx
		mov 	[kernel_buffer + 4],  ebx
		mov 	[kernel_buffer],      eax
		
		mov eax, esp								; Save ESP, get Kernel's one.
		push eax
			call scheduler_save_esp
		pop eax
		
		call scheduler_get_temp_esp
		mov esp, eax
		
		call int_80									; Make the syscall
		
		call scheduler_load_esp
		mov esp,eax
		
		popad
		pop		es
		pop		ds
		sti
		
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
		mov 	eax, 7				; eax en 8 para pcreate
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
		
openfifo:
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
		mov		eax, 19				; eax en 18 para pgetpid_at
		mov 	ebx, [ebp+8]		; signal
		mov 	ecx, [ebp+12]		; pid
		int		80h
		popa
		mov		esp, ebp
		pop		ebp
		mov		eax, [kernel_buffer + 60]
		ret

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



