OUTPUT_FORMAT(binary)               /* Binary output */
OUTPUT_ARCH(i386:x86-64)            /* x86-64 architecture */

start_address = 0x400000 + 0x1000;           /* Start address */

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
		SHORT(header_size - prog)   /* Program header size */
		SHORT(1)                    /* Number of program headers */
		SHORT(0)                    /* Section header entries */
		LONG(0)                     /* Section header string index */
		
		/* Program Header */
		prog = .;                   /* Current location (program start) */
		LONG(1)                     /* Loadable segment */
		QUAD(7)                     /* RWX flags */
		LONG(0)                     /* File offset */
		QUAD(start_address)         /* Virtual address */
		QUAD(start_address)         /* Physical address */
		QUAD(filesize)              /* File size */
		QUAD(memsize)               /* Memory size (header + .data + .bss) */
		QUAD(8)                     /* Alignment */
		header_size = .;            /* End of header */
	}
	
	/* Set the VMA for the following sections */
	. = start_address + header_size;
	
	/* Code and Data Sections */
	.data : AT(header_size) {
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
