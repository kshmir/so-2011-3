ENTRY (_loader)

SECTIONS{
	. = 0x00100000;

	.text :{
		*(.text)
	}

	.data : {
		*(.__mbHeader)
	}

	.rodata ALIGN (0x1000) : {
		*(.rodata)
	}

	.bss : {
		_sbss = .;
		*(COMMON)
		*(.bss)
		_ebss = .;
	}
}
