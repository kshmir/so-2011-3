;
;	Loader.asm --- Starts up the system with GRUB.
;

global _loader		; making entry point visible to linker
global eokl		; end of kernel land
extern kmain		; _main is defined elsewhere

; setting up the Multiboot header - see GRUB docs for details

MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required

align 4
SECTION .text	
MultiBootHeader:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
	; reserve initial kernel stack space
	STACKSIZE equ 0x4000		; that's 16k.
section .text
	_loader:
	mov esp, stack+STACKSIZE; set up the stack
	push eax		; pass Multiboot magic number
	push ebx		; pass Multiboot info structure
	call  kmain		; call kernel proper
	hlt			; halt machine should kernel return

eokl	dd STACKSIZE + stack
	section .bss
	align 32
	stack:
	resb STACKSIZE		; reserve 16k stack on a quadword boundary
	

	