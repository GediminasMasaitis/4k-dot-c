OUTPUT_FORMAT(binary)               /* Binary output */
OUTPUT_ARCH(i386:x86-64)            /* x86-64 architecture */

pos = 0x300000;                     /* Start address */

SECTIONS
{
	header :                         /* ELF header */
	{
		QUAD(0x00010102464C457F)     /* ELF magic number */
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
		prog = . ;                  /* Current location (program start) */
		LONG(1)                     /* Loadable segment */
		QUAD(7)                     /* RWX flags */
		LONG(0)                     /* File offset */
		QUAD(pos)                   /* Virtual address */
		QUAD(pos)                   /* Physical address */
		QUAD(filesize)              /* File size */
		QUAD(0x10000000)            /* Memory size */
		QUAD(8)                     /* Alignment */
		size = . ;                  /* Current location (end of header) */
	}
	
	data = . ;                      /* Current location (data start) */
	.data (data + pos) : AT(data)   /* Data section at memory + file offset */
	{
		*(.rodata) *(.rodata.*)    /* Include .rodata sections */
		*(.data) *(.data.*)        /* Include .data sections */
		*(.bss) *(.bss.*)          /* Include .bss sections */
		*(.text) *(.text.*)        /* Include .text sections */
	}

	filesize = . - pos ;           /* Current location (end of file) */

	.payload 0x400000 (NOLOAD) :
	{
		*(.payload) *(.payload.*)
	}
	
	/DISCARD/ : { *(*) }            /* Discard others */

}
