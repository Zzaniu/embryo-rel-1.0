
OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradbigmips",
	      	  "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = STARTADDR;
	.text :
	{
		_ftext = . ;
		*(.text)
		*(.rodata)
		*(.reginfo)
	} = 0
	_etext = . ;
	PROVIDE(etext = .);				/* 在start.S中用于计算代码拷贝范围 */
	.data :
	{
		_fdata = . ;
		*(.data)
		. = ALIGN(32);
		*(.data.align32)
		. = ALIGN(64);
		*(.data.align64)
		. = ALIGN(128);
		*(.data.align128)
		. = ALIGN(4096);
		*(.data.align4096)
		CONSTRUCTORS
	} 	

	.ctors :
	{
		 __CTOR_LIST__ = .;
		 LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		 LONG(0)
		 __CTOR_END__ = .;
	}
	.dtors :
	{
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;
	}
	_gp = ALIGN(16) + 0x7ff0;		/* 在start.S中被设置到gp寄存器 */
	.sdata : {*(.sdata)}
	_edata = . ;
	PROVIDE(edata = .);				/* 在start.S中用于计算代码拷贝范围 */
	_bss_start = . ;
	.sbss : {*(.sbss)}
	.bss : 
	{
		*(.dynbss)
		*(.bss)
		. = ALIGN(32);
		*(.bss.align32)
		. = ALIGN(64);
		*(.bss.align64)
		. = ALIGN(128);
		*(.bss.align128)
		. = ALIGN(4096);
		*(.bss.align4096)
		*(COMMON)
	}
	_end = . ;
	PROVIDE(end = .);				/* 在start.S中用于计算代码拷贝范围 */

}

