OUTPUT_FORMAT(binary)               /* Binary output */
OUTPUT_ARCH(i386:x86-64)            /* x86-64 architecture */

start_address = 0x400000 + 0x80;           /* Start address */

SECTIONS
{
	/* Set the VMA for the following sections */
	. = start_address;
	
	/* Code and Data Sections */
	.data : {
		*(.rodata) *(.rodata.*)
		*(.data) *(.data.*)
		*(.text) *(.text.*)
	}
	
	/* Compute file size: header size plus size of .data */
	filesize = (. - start_address);
	
	/* BSS Section: Allocated in memory but not in file */
	.bss (NOLOAD) : {
		bss_start = .;
		*(.bss) *(.bss.*)
	}
	
	memsize = . - start_address; /* Compute total memory size */
	
	/DISCARD/ : { *(*) } /* Discard unneeded sections */
}
