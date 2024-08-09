OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)

pos = 0x08048000 ;

SECTIONS
{
	header :
	{
		QUAD(0x00010101464C457F)
		QUAD(0)
		SHORT(2)
		SHORT(3)
		LONG(1)
		LONG(_start)
		LONG(0x34)
		QUAD(0)
		SHORT(prog)
		SHORT(size - prog)
		SHORT(1)
		SHORT(0)
		LONG(0)
		prog = . ;
		QUAD(1)
		LONG(pos)
		LONG(pos)
		LONG(filesize)
		LONG(filesize)
		LONG(7)
		LONG(4)
		size = . ;
	}
	
	data = . ;
	.data (data + pos) : AT(data)
	{
		*(.text) *(.text.*)
		*(.rodata) *(.rodata.*)
		*(.bss) *(.bss.*)
		*(.data) *(.data.*)
	}
	
	/DISCARD/ : { *(*) }
	
	filesize = . ;
}
