
OUTPUT_FORMAT("elf64-tradlittlemips", "elf64-tradbigmips",
           "elf64-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(main)
SECTIONS
{
	. = 0xffffffff80200000 ;

	.text :
	{
		test.o(.text);
		*.o(.text);
	}	
	.align = 4;
}
