OUTPUT_FORMAT(binary)               /* Binary output */
OUTPUT_ARCH(i386:x86-64)            /* x86-64 architecture */

start_address = 0x400000;  /* Start address */

SECTIONS
{
	. = start_address;
	
	.data : {
		*(.first)
		*(.text) *(.text.*)
		*(.rodata) *(.rodata.*)
		*(.data) *(.data.*)
	}
	
	.bss (NOLOAD) : {
		*(.bss)
		*(COMMON)
		*(SORT_BY_NAME(.bss.*))
	}
	
	/DISCARD/ : { *(*) } /* Discard unneeded sections */
}
