OUTPUT_FORMAT(binary)               /* Binary output */
OUTPUT_ARCH(i386:x86-64)            /* x86-64 architecture */

pos = 0x400000;                     /* Start address */

SECTIONS
{
	/* ELF Header */
	header : {
		QUAD(0x00010102464C457F)    /* ELF magic number */
		QUAD(0)                     /* Reserved */
		SHORT(2)                    /* Executable type */
		SHORT(0x3E)                 /* x86-64 machine */
		LONG(1)                     /* ELF version */
		QUAD(_start)                /* Entry point */
		QUAD(0x40)                  /* Program header offset */
		QUAD(0)                     /* Section header offset */
		LONG(0)                     /* Flags */
		SHORT(prog)                 /* Header size */
		SHORT(size - prog)          /* Program header size */
		SHORT(1)                    /* Number of program headers */
		SHORT(0)                    /* Section header entries */
		LONG(0)                     /* Section header string index */
		
		/* Program Header */
		prog = .;                   /* Current location (program start) */
		LONG(1)                     /* Loadable segment */
		QUAD(7)                     /* RWX flags */
		LONG(0)                     /* File offset */
		QUAD(pos)                   /* Virtual address */
		QUAD(pos)                   /* Physical address */
		QUAD(filesize)              /* File size */
		QUAD(memsize)               /* Memory size (header + .data + .bss) */
		QUAD(8)                     /* Alignment */
		size = .;                   /* End of header */
	}
	
	/* Save header size */
	header_size = size;
	
	/* Set the VMA for the following sections */
	. = pos + header_size;
	
	/* Code and Data Sections */
	.data : AT(header_size) {
		*(.text) *(.text.*)
		*(.rodata) *(.rodata.*)
		*(.data) *(.data.*)
	}
	data_end = .;  /* End of .data section */
	
	/* Compute file size: header size plus size of .data */
	filesize = header_size + (data_end - (pos + header_size));
	
	/* BSS Section: Allocated in memory but not in file */
	.bss (NOLOAD) : {
		. = ALIGN(16);
		bss_start = .;
		*(.bss) *(.bss.*)
		. = ALIGN(16);
		bss_end = .;
	}
	
	/* Compute total memory size */
	memsize = . - pos;
	
	/DISCARD/ : { *(*) } /* Discard unneeded sections */
}
