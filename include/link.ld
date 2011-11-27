ENTRY (_loader)

SECTIONS{
	. = 0x00100000;

	.data : {
		*(.__mbHeader)
	}

	.text :{
		*(.text)
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
