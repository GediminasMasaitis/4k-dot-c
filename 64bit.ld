OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:x86-64)

pos = 0x400000 ;

SECTIONS
{
	header :
	{
		QUAD(0x00010102464C457F)
 		QUAD(0)
 		SHORT(2)
		SHORT(0x3E)
 		LONG(1)
		QUAD(_start + pos)
		QUAD(0x40)
 		QUAD(0)
		LONG(0)
		SHORT(prog)
		SHORT(size - prog)
 		SHORT(1)
 		SHORT(0)
 		LONG(0)
		prog = . ;
		LONG(1)
		QUAD(7)
		LONG(0)
		QUAD(pos)
		QUAD(pos)
		QUAD(filesize)
		QUAD(filesize)
		QUAD(8)
		size = . ;
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
	
	/DISCARD/ : { *(*) }
	
	filesize = . ;
}
