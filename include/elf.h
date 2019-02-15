
/**
 * Copyright(c) 2019-1-16 Shangwen Wu	
 *
 * ELF文件格式定义 
 * 
 */

#ifndef __ELF_H__
#define __ELF_H__

#include <sys/types.h>

/* ELF头字段定义  */
#define EI_NIDENT		16				/* ELF标识信息字节数组长度 */

/* 目标文件标识索引 */
#define EI_MAG0			0				/* 文件格式标识 */
#define EI_MAG1			1				/* 文件格式标识 */
#define EI_MAG2			2				/* 文件格式标识 */
#define EI_MAG3			3				/* 文件格式标识 */
#define EI_CLASS		4				/* 文件位宽类型 */
#define EI_DATA			5				/* 文件数据大小端 */
#define EI_VERSION		6				/* 文件版本 */
#define EI_PAD			7				/* 补齐字节开始处 */

/* EI_MAG0-3 */
#define EI_MAG0_VAL		0x7f			/* 固定魔术字 */
#define EI_MAG1_VAL		'E'				
#define EI_MAG2_VAL		'L'
#define EI_MAG3_VAL		'F'

/* EI_CLASS */
#define ELFCLASSNONE	0				/* 非法类别 */
#define ELFCLASS32		1				/* 32位目标文件 */
#define ELFCLASS64		2				/* 64位目标文件 */

/* EI_DATA */
#define ELFDATANONE		0				/* 非法数据编码 */
#define ELFDATA2LSB		1				/* 小端 */
#define ELFDATA2MSB		2				/* 大端 */

/* e_type */
#define ET_NONE			0				/* 非法目标文件类型 */
#define ET_REL			1				/* 可重定位文件 */
#define ET_EXEC			2				/* 可执行文件 */
#define ET_DYN			3				/* 共享目标文件 */
#define ET_CORE			4				/* Core文件 */
#define ET_LOPROC		0xff00			/* 特定处理器文件 */
#define ET_HIPROC		0xffff			/* 特定处理器文件 */

/* e_machine */
#define EM_NONE			0				/* 未指定 */
#define EM_M32			1				/* AT&T WE 32100 */
#define EM_SPARC		2				/* SPARC */
#define EM_386			3				/* Intel 80386 */
#define EM_68K			4				/* Motorola 6800 */
#define EM_88K			5				/* Motorola 8800 */
#define EM_860			7				/* Intel 80860 */
#define EM_MIPS			8				/* MIPS RS3000 */

/* EI_VERSION & e_version */
#define EV_NONE			0				/* 非法版本 */
#define EV_CURRENT		1				/* 目前唯一合法值 */

/* 节区头部字段定义 */
/* sh_type */
#define SHT_NULL		0				/* 无效节区头部 */
#define SHT_PROGBITS	1				/* 节区由程序定义 */
#define SHT_SYMTAB		2				/* 符号表节区 */
#define SHT_STRTAB		3				/* 字符串表节区 */
#define SHT_RELA		4				/* 对齐模式的重定位表节区 */
#define SHT_HASH		5				/* 哈希表节区 */
#define SHT_DYNAMIC		6				/* 节区包含动态链接信息 */
#define SHT_NOTE		7				/* 节区包含某些文件信息 */
#define SHT_NOBITS		8				/* 节区不占用文件空间 */
#define SHT_REL			9				/* 非对齐模式的重定位表节区 */
#define SHT_SHLIB		10				/* 保留节区 */
#define SHT_DYNSYM		11				/* 节区包含必要的动态链接符号 */
#define SHT_LOPROC		0x70000000		/* 保留给处理器专用 */
#define SHT_HIPROC		0x7fffffff		/* 保留给处理器专用 */
#define SHT_LOUSER		0x80000000		/* 保留给应用程序 */
#define SHT_HIUSER		0x8fffffff		/* 保留给应用程序 */

/* sh_flags */
#define SHF_WRITE		0x1				/* 节区包含执行过程中可写的数据 */
#define SHF_ALLOC		0x2				/* 节区在执行过程中占用内存 */
#define SHF_EXECINSTR	0x4				/* 节区包含可执行的机器指令 */
#define SHF_MASKPROC	0xf0000000		/* 保留给处理器专用 */

/* 程序头部字段定义 */
/* p_type */
#define PT_NULL			0				/* 无效程序段 */
#define PT_LOAD			1				/* 可加载程序段 */
#define PT_DYNAMIC		2				/* 程序段包含动态链接信息 */
#define PT_INTERP		3				/* 用于解释器调用的字符串 */
#define PT_NOTE			4				/* 程序段包含附加信息的位置和大小等信息 */
#define PT_SHLIB		5				/* 保留 */
#define PT_PHDR			6				/* 程序段包含程序头表自身大小和位置信息 */
#define PT_LOPROC		0x70000000		/* 保留给应用程序 */
#define PT_HIPROC		0x7fffffff		/* 保留给应用程序 */

/* ELF32 */
typedef u32 Elf32_Addr;
typedef u32 Elf32_Word;
typedef s32 Elf32_SWord;
typedef u32 Elf32_Off;
typedef u16 Elf32_Half;
typedef s16 Elf32_SHalf;

/* elf32文件头 */
typedef struct elf32_ehdr {
	u8 e_ident[EI_NIDENT];				/* 目标文件头标识 */
	Elf32_Half e_type;					/* 目标文件头类型 */
	Elf32_Half e_machine;				/* 文件的目标体系结构类型 */
	Elf32_Word e_version;				/* 目标文件版本 */
	Elf32_Addr e_entry;					/* 程序入口的虚拟地址 */
	Elf32_Off e_phoff;					/* 程序头表偏移量 */
	Elf32_Off e_shoff;					/* 节区头表偏移量 */
	Elf32_Word e_flags;					/* 特定于处理器的标志 */
	Elf32_Half e_ehsize;				/* ELF头部大小 */
	Elf32_Half e_phentsize;				/* 程序头表的表项大小 */
	Elf32_Half e_phnum;					/* 程序头表的表项个数 */
	Elf32_Half e_shentsize;				/* 节区头表的表项大小 */
	Elf32_Half e_shnum;					/* 节区头表的表项个数 */
	Elf32_Half e_shstrndx;				/* 节区名称字符串的表项索引 */
} Elf32_Ehdr;

/* elf32节区头部 */
typedef struct elf32_shdr {
	Elf32_Word sh_name;					/* 对应节区名称字符串表节区的索引 */
	Elf32_Word sh_type;					/* 节区类型 */
	Elf32_Word sh_flags;				/* 节区标志 */
	Elf32_Addr sh_addr;					/* 如果节区需要加载到内存，此字段给出加载的内存起始位置 */
	Elf32_Off sh_offset;				/* 节区在文件的偏移位置 */
	Elf32_Word sh_size;					/* 节区大小 */
	Elf32_Word sh_link;					/* 节区头表索引，依赖于节区类型 */
	Elf32_Word sh_info;					/* 节区附加信息，依赖于节区类型 */
	Elf32_Word sh_addralign;			/* 节区对按多少字节对齐 */
	Elf32_Word sh_entsize;				/* 如果节区包含固定大小的项目，则该字段表示项目大小 */
} Elf32_Shdr;

/* elf32程序头部 */
typedef struct elf32_phdr {
	Elf32_Word p_type;					/* 程序段类型 */
	Elf32_Off p_offset;					/* 程序段相对于文件的偏移 */
	Elf32_Addr p_vaddr;					/* 程序段加载内存虚拟地址 */
	Elf32_Addr p_paddr;					/* 程序段加载内存物理地址，仅用于某些物理地址相关的系统 */
	/* 注意：memsz可能大于filesz，大于的部分需要清零 */
	Elf32_Word p_filesz;				/* 此程序段在文件中占用的字节数据 */
	Elf32_Word p_memsz;					/* 此程序段在内存镜像中占用的字节数据 */
	Elf32_Word p_flags;					/* 段相关标识 */
	Elf32_Word p_align;					/* 可加载段的对齐要求 */
} Elf32_Phdr;


/* ELF64 */
typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u64 Elf64_XWord;
typedef s64 Elf64_SXWord;
typedef u32 Elf64_Word;
typedef s32 Elf64_SWord;
typedef u16 Elf64_Half;
typedef s16 Elf64_SHalf;

/* elf64文件头 */
typedef struct elf64_ehdr {
	u8 e_ident[EI_NIDENT];				/* 目标文件头标识 */
	Elf64_Half e_type;					/* 目标文件头类型 */
	Elf64_Half e_machine;				/* 文件的目标体系结构类型 */
	Elf64_Word e_version;				/* 目标文件版本 */
	Elf64_Addr e_entry;					/* 程序入口的虚拟地址 */
	Elf64_Off e_phoff;					/* 程序头表偏移量 */
	Elf64_Off e_shoff;					/* 节区头表偏移量 */
	Elf64_Word e_flags;					/* 特定于处理器的标志 */
	Elf64_Half e_ehsize;				/* ELF头部大小 */
	Elf64_Half e_phentsize;				/* 程序头表的表项大小 */
	Elf64_Half e_phnum;					/* 程序头表的表项个数 */
	Elf64_Half e_shentsize;				/* 节区头表的表项大小 */
	Elf64_Half e_shnum;					/* 节区头表的表项个数 */
	Elf64_Half e_shstrndx;				/* 节区名称字符串的表项索引 */
} Elf64_Ehdr;

/* elf64节区头部 */
typedef struct elf64_shdr {
	Elf64_Word sh_name;					/* 对应节区名称字符串表节区的索引 */
	Elf64_Word sh_type;					/* 节区类型 */
	Elf64_XWord sh_flags;				/* 节区标志 */
	Elf64_Addr sh_addr;					/* 如果节区需要加载到内存，此字段给出加载的内存起始位置 */
	Elf64_Off sh_offset;				/* 节区在文件的偏移位置 */
	Elf64_XWord sh_size;				/* 节区大小 */
	Elf64_Word sh_link;					/* 节区头表索引，依赖于节区类型 */
	Elf64_Word sh_info;					/* 节区附加信息，依赖于节区类型 */
	Elf64_XWord sh_addralign;			/* 节区对按多少字节对齐 */
	Elf64_XWord sh_entsize;				/* 如果节区包含固定大小的项目，则该字段表示项目大小 */
} Elf64_Shdr;

/* elf64程序头部 */
typedef struct elf64_phdr {
	Elf64_Word p_type;					/* 程序段类型 */
	Elf64_Word p_flags;					/* 段相关标识 */
	Elf64_Off p_offset;					/* 程序段相对于文件的偏移 */
	Elf64_Addr p_vaddr;					/* 程序段加载内存虚拟地址 */
	Elf64_Addr p_paddr;					/* 程序段加载内存物理地址，仅用于某些物理地址相关的系统 */
	/* 注意：memsz可能大于filesz，大于的部分需要清零 */
	Elf64_XWord p_filesz;				/* 此程序段在文件中占用的字节数据 */
	Elf64_XWord p_memsz;				/* 此程序段在内存镜像中占用的字节数据 */
	Elf64_XWord p_align;				/* 可加载段的对齐要求 */
} Elf64_Phdr;

#endif //__ELF_H__
