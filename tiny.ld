OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

pos = 0x08048000 ;

SECTIONS
{
	start = . ;
	header :
	{
		ehdr = . ;
		QUAD(0x00010101464C457F)
		QUAD(0)
		SHORT(2)
		SHORT(3)
		LONG(1)
		LONG(_start + pos)
		LONG(0x34)
		QUAD(0)
		SHORT(ehdrsize)
		SHORT(phdrsize)
		SHORT(1)
		SHORT(0)
		LONG(0)
		phdr = . ;
		ehdrsize = phdr - ehdr ;
		QUAD(1)
		LONG(pos)
		LONG(pos)
		LONG(filesize)
		LONG(filesize)
		LONG(7)
		LONG(0x1000)
		phdrsize = . - phdr ;
	}
	
	.text : { *(.text) *(.text.*) }
	
	.data
		(ADDR(.text) + SIZEOF(.text) + pos)
		: AT(ADDR(.text) + SIZEOF(.text))
	{
		*(.rodata) *(.rodata.*)
		*(.bss) *(.bss.*)
		*(.data) *(.data.*)
	}
	
	/DISCARD/ : { * }
	
	filesize = . ;
}
