
/**
 * Copyright(c) 2019-1-17 Shangwen Wu 
 *
 * ELF文件加载器
 *
 * 注意：暂不支持超过4GB文件的访问
 */
#include <common.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <command.h>
#include <load_exec.h>
#include <elf.h>
#include <fs/termio.h>

/* elf调试信息开关 */
#define ELF_DEBUG	1
#if ELF_DEBUG
#define ELF_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define ELF_DBG(fmt, args...)		do{}while(0)	
#endif

#define ELF_FMT_NOELF	0		/* 非ELF格式文件 */
#define ELF_FMT_ELF32	1		/* ELF32文件 */
#define ELF_FMT_ELF64	2		/* ELF64文件 */
#define ELF_FMT_BADELF	3		/* 不支持或非法ELF格式 */

/* 调试用 */
#if ELF_DEBUG

/**
 * 描述：打印ELF标识信息
 */
static void dump_elf_ident(uint8_t *elf_ident)
{
	ELF_DBG("ELF Ident:\n");
	ELF_DBG("%-*s0x%x\n", 40, "Magic[0]", elf_ident[EI_MAG0]);
	ELF_DBG("%-*s0x%x\n", 40, "Magic[1]", elf_ident[EI_MAG1]);
	ELF_DBG("%-*s0x%x\n", 40, "Magic[2]", elf_ident[EI_MAG2]);
	ELF_DBG("%-*s0x%x\n", 40, "Magic[3]", elf_ident[EI_MAG3]);
	ELF_DBG("%-*s", 40, "Class");
	if(ELFCLASS64 == elf_ident[EI_CLASS])
		ELF_DBG("ELF64\n");
	else if(ELFCLASS32 == elf_ident[EI_CLASS])
		ELF_DBG("ELF32\n");
	else 
		ELF_DBG("None\n");
	ELF_DBG("%-*s", 40, "Data");
	if(ELFDATA2LSB == elf_ident[EI_DATA])
		ELF_DBG("Little endian\n");
	else if(ELFDATA2MSB == elf_ident[EI_DATA])
		ELF_DBG("Bid endian\n");
	else 
		ELF_DBG("None\n");
	ELF_DBG("%-*s", 40, "Version");
	if(EV_CURRENT == elf_ident[EI_VERSION])
		ELF_DBG("Current\n");
	else 
		ELF_DBG("None\n");
}

/**
 * 描述：打印ELF32头信息
 */
static void dump_elf32_ehdr(struct elf32_ehdr *ehdr)
{
	ELF_DBG("ELF Header:\n");
	ELF_DBG("%-*s", 40, "Type");
	switch(ehdr->e_type) {
		case ET_EXEC:
			ELF_DBG("Exec\n");
			break;
		case ET_REL:
			ELF_DBG("Rel\n");
			break;
		case ET_DYN:
			ELF_DBG("Dyn\n");
			break;
		case ET_CORE:
			ELF_DBG("Core\n");
			break;
		case ET_LOPROC: case ET_HIPROC:
			ELF_DBG("Processor spec\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s", 40, "Arch");
	switch(ehdr->e_machine) {
		case EM_MIPS:
			ELF_DBG("MIPS RS3000\n");
			break;
		case EM_M32:
			ELF_DBG("AT&T WE 32100\n");
			break;
		case EM_SPARC:
			ELF_DBG("SPARC\n");
			break;
		case EM_386:
			ELF_DBG("Intel 80386\n");
			break;
		case EM_68K:
			ELF_DBG("Motorola 6800\n");
			break;
		case EM_88K:
			ELF_DBG("Motorola 8800\n");
			break;
		case EM_860:
			ELF_DBG("Intel 80860\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s", 40, "Version");
	if(EV_CURRENT == ehdr->e_version)
		ELF_DBG("Current\n");
	else 
		ELF_DBG("None\n");
	ELF_DBG("%-*s0x%x\n", 40, "Entry", ehdr->e_entry);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Tab Offs", ehdr->e_phoff);
	ELF_DBG("%-*s0x%x\n", 40, "Section Header Tab Offs", ehdr->e_shoff);
	ELF_DBG("%-*s0x%x\n", 40, "Flags", ehdr->e_flags);
	ELF_DBG("%-*s0x%x\n", 40, "ELF Header size", ehdr->e_ehsize);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Tab Ent Size", ehdr->e_phentsize);
	ELF_DBG("%-*s%d\n", 40, "Program Header Tab Ent Count", ehdr->e_phnum);
	ELF_DBG("%-*s0x%x\n", 40, "Section Header Tab Ent Size", ehdr->e_shentsize);
	ELF_DBG("%-*s%d\n", 40, "Section Header Tab Ent Count", ehdr->e_shnum);
	ELF_DBG("%-*s0x%x\n", 40, "Section'Name String Sec Index", ehdr->e_shstrndx);
}

/**
 * 描述：打印ELF32程序头信息
 */
static void dump_elf32_phdr(struct elf32_phdr *phdr)
{
	ELF_DBG("ELF Program Header:\n");
	ELF_DBG("%-*s", 40, "Type");
	switch(phdr->p_type) {
		case PT_LOAD:
			ELF_DBG("Load\n");
			break;
		case PT_DYNAMIC:
			ELF_DBG("Dynamic\n");
			break;
		case PT_INTERP:
			ELF_DBG("Interp\n");
			break;
		case PT_NOTE:
			ELF_DBG("Note\n");
			break;
		case PT_SHLIB:
			ELF_DBG("reservered\n");
			break;
		case PT_PHDR:
			ELF_DBG("Phdr\n");
			break;
		case PT_LOPROC: case PT_HIPROC:
			ELF_DBG("Processor spec\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s0x%x\n", 40, "Flags", phdr->p_flags);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Offs", phdr->p_offset);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Mem Addr(V)", phdr->p_vaddr);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Mem Addr(P)", phdr->p_paddr);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header File Size", phdr->p_filesz);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Mem Size", phdr->p_memsz);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Align", phdr->p_align);
}

/**
 * 描述：打印ELF64头信息
 */
static void dump_elf64_ehdr(struct elf64_ehdr *ehdr)
{
	ELF_DBG("ELF Header:\n");
	ELF_DBG("%-*s", 40, "Type");
	switch(ehdr->e_type) {
		case ET_EXEC:
			ELF_DBG("Exec\n");
			break;
		case ET_REL:
			ELF_DBG("Rel\n");
			break;
		case ET_DYN:
			ELF_DBG("Dyn\n");
			break;
		case ET_CORE:
			ELF_DBG("Core\n");
			break;
		case ET_LOPROC: case ET_HIPROC:
			ELF_DBG("Processor spec\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s", 40, "Arch");
	switch(ehdr->e_machine) {
		case EM_MIPS:
			ELF_DBG("MIPS RS3000\n");
			break;
		case EM_M32:
			ELF_DBG("AT&T WE 32100\n");
			break;
		case EM_SPARC:
			ELF_DBG("SPARC\n");
			break;
		case EM_386:
			ELF_DBG("Intel 80386\n");
			break;
		case EM_68K:
			ELF_DBG("Motorola 6800\n");
			break;
		case EM_88K:
			ELF_DBG("Motorola 8800\n");
			break;
		case EM_860:
			ELF_DBG("Intel 80860\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s", 40, "Version");
	if(EV_CURRENT == ehdr->e_version)
		ELF_DBG("Current\n");
	else 
		ELF_DBG("None\n");
	ELF_DBG("%-*s0x%llx\n", 40, "Entry", ehdr->e_entry);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Tab Offs", ehdr->e_phoff);
	ELF_DBG("%-*s0x%llx\n", 40, "Section Header Tab Offs", ehdr->e_shoff);
	ELF_DBG("%-*s0x%x\n", 40, "Flags", ehdr->e_flags);
	ELF_DBG("%-*s0x%x\n", 40, "ELF Header size", ehdr->e_ehsize);
	ELF_DBG("%-*s0x%x\n", 40, "Program Header Tab Ent Size", ehdr->e_phentsize);
	ELF_DBG("%-*s%d\n", 40, "Program Header Tab Ent Count", ehdr->e_phnum);
	ELF_DBG("%-*s0x%x\n", 40, "Section Header Tab Ent Size", ehdr->e_shentsize);
	ELF_DBG("%-*s%d\n", 40, "Section Header Tab Ent Count", ehdr->e_shnum);
	ELF_DBG("%-*s0x%x\n", 40, "Section'Name String Sec Index", ehdr->e_shstrndx);
}

/**
 * 描述：打印ELF64程序头信息
 */
static void dump_elf64_phdr(struct elf64_phdr *phdr)
{
	ELF_DBG("ELF Program Header:\n");
	ELF_DBG("%-*s", 40, "Type");
	switch(phdr->p_type) {
		case PT_LOAD:
			ELF_DBG("Load\n");
			break;
		case PT_DYNAMIC:
			ELF_DBG("Dynamic\n");
			break;
		case PT_INTERP:
			ELF_DBG("Interp\n");
			break;
		case PT_NOTE:
			ELF_DBG("Note\n");
			break;
		case PT_SHLIB:
			ELF_DBG("reservered\n");
			break;
		case PT_PHDR:
			ELF_DBG("Phdr\n");
			break;
		case PT_LOPROC: case PT_HIPROC:
			ELF_DBG("Processor spec\n");
			break;
		default:
			ELF_DBG("None\n");
	}
	ELF_DBG("%-*s0x%x\n", 40, "Flags", phdr->p_flags);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Offs", phdr->p_offset);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Mem Addr(V)", phdr->p_vaddr);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Mem Addr(P)", phdr->p_paddr);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header File Size", phdr->p_filesz);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Mem Size", phdr->p_memsz);
	ELF_DBG("%-*s0x%llx\n", 40, "Program Header Align", phdr->p_align);
}

/**
 * 描述：计算校验和，算法直接使用网络常用的16位累加
 * 注意：长度不能超过long容纳范围
 */
static u16 __calc_chksum(u16 *buf, long len)
{
	u32 sum = 0;
	u16 *pos = buf;
	while(len - 2 >= 0) {		
		sum += *pos++;
		len -= 2;
	}
	if(1 == len)
		sum += *((u8 *)pos);
	sum = (sum >> 16) + (sum & 0xffff);
	while(sum >> 16) 
		sum += (sum >> 16);
	
	return (u16)~sum;
}

#endif

/**
 * 检查当前段加载内存区域是否合法
 */
static int __elf32_chkloadaddr(struct elf32_phdr *phdr)
{
	extern int md_load_addr_isvalid(void *, void *);		//defined in mipsdep.c
	if(md_load_addr_isvalid((void *)(phdr->p_vaddr), 
				(void *)(phdr->p_vaddr + phdr->p_memsz)))	
		return 0;

	return -1;
}

static int __elf64_chkloadaddr(struct elf64_phdr *phdr)
{
	extern int md_load_addr_isvalid(void *, void *);		//defined in mipsdep.c
	if(md_load_addr_isvalid((void *)(ulong)(phdr->p_vaddr), 
				(void *)(ulong)(phdr->p_vaddr + phdr->p_memsz)))	
		return 0;

	return -1;
}

/**
 * 描述：将指定程序段拷贝到内存区域
 */
static int __elf32_loadseg2mem(int fd, struct elf32_phdr *phdr)
{
	if(-1 == lseek(fd, phdr->p_offset, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		return -1;	
	}
	if(read(fd, (void *)(phdr->p_vaddr), phdr->p_filesz) != phdr->p_filesz) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		return -1;
	}
	printf("Load addr: 0x%x, Load size: 0x%x\n", phdr->p_vaddr, phdr->p_filesz);
	
#if ELF_DEBUG
	ELF_DBG("Image chksum: 0x%x\n", __calc_chksum((u16 *)(ulong)(phdr->p_vaddr), (long)phdr->p_filesz));
#endif

	return 0;
}

static int __elf64_loadseg2mem(int fd, struct elf64_phdr *phdr)
{
	if(-1 == lseek(fd, (off_t)phdr->p_offset, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		return -1;	
	}
	/* 注意：文件不能过大 */
	if(read(fd, (void *)(ulong)(phdr->p_vaddr), (size_t)phdr->p_filesz) != (size_t)phdr->p_filesz) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		return -1;
	}
	printf("Load addr: 0x%llx, Load size: 0x%llx\n", phdr->p_vaddr, phdr->p_filesz);
	
#if ELF_DEBUG
	ELF_DBG("Image chksum: 0x%x\n", __calc_chksum((u16 *)(ulong)(phdr->p_vaddr), (long)phdr->p_filesz));
#endif

	return 0;
}

/**
 * 描述：清零程序段剩余区域
 */
static void __elf32_clearseg(struct elf32_phdr *phdr)
{
	/* 如果p_memsz大于p_filesz，则需要清零从p_filesz到p_memsz的区域 */
	if(phdr->p_memsz > phdr->p_filesz) {
		bzero((void *)(phdr->p_vaddr + phdr->p_filesz), phdr->p_memsz - phdr->p_filesz);
		printf("Clr addr: 0x%x, Clr size: 0x%x\n", 
				phdr->p_vaddr + phdr->p_filesz, phdr->p_memsz - phdr->p_filesz);
	}
}

static void __elf64_clearseg(struct elf64_phdr *phdr)
{
	/* 如果p_memsz大于p_filesz，则需要清零从p_filesz到p_memsz的区域 */
	if(phdr->p_memsz > phdr->p_filesz) {
		bzero((void *)(ulong)(phdr->p_vaddr + phdr->p_filesz), (size_t)(phdr->p_memsz - phdr->p_filesz));
		printf("Clr addr: 0x%llx, Clr size: 0x%llx\n", 
				phdr->p_vaddr + phdr->p_filesz, phdr->p_memsz - phdr->p_filesz);
	}
}

/**
 * 描述：检查ELF文件标识，暂不支持压缩格式
 */
static int __check_elf_ident(uint8_t *elf_ident)
{
#if ELF_DEBUG
	dump_elf_ident(elf_ident);
#endif

	/* check magic */
	if(elf_ident[EI_MAG0] != EI_MAG0_VAL ||
			elf_ident[EI_MAG1] != EI_MAG1_VAL ||
			elf_ident[EI_MAG2] != EI_MAG2_VAL ||
			elf_ident[EI_MAG3] != EI_MAG3_VAL) 
		return ELF_FMT_NOELF;
	
	/* check endian */
#if BYTE_ORDER == LITTLE_ENDIAN
	if(elf_ident[EI_DATA] != ELFDATA2LSB)
		return ELF_FMT_BADELF;
#elif BYTE_ORDER == LITTLE_ENDIAN
	if(elf_ident[EI_DATA] != ELFDATA2MSB)
		return ELF_FMT_BADELF;
#endif

	/* check ELF version */
	if(elf_ident[EI_VERSION] != EV_CURRENT)
		return ELF_FMT_BADELF;
	
	/* check file class */
	if(ELFCLASS64 == elf_ident[EI_CLASS])
		return ELF_FMT_ELF64;
	else if(ELFCLASS32 == elf_ident[EI_CLASS])
		return ELF_FMT_ELF32;

	return ELF_FMT_BADELF;
}

static int __elf64_loader(int fd, int flags, unsigned long *entry)
{
	unsigned long i;
	Elf64_Off lowest_off;
	struct elf64_ehdr *elfhdr = NULL;
	struct elf64_phdr *ephdrtab = NULL, *ephdr;

	/* 获取ELF格式头 */
	if(!(elfhdr = (struct elf64_ehdr *)malloc(sizeof(*elfhdr)))) {
		fprintf(stderr, "%s: cannot allocate memory for ELF header\n", __func__);
		return EXEC_LOAD_RES_LOADERR;
	}
	if(-1 == lseek(fd, 0UL, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		goto load_err;
	}
	if(read(fd, (uint8_t *)elfhdr, sizeof(*elfhdr)) != sizeof(*elfhdr)) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		goto load_err;
	}
#if ELF_DEBUG
	dump_elf64_ehdr(elfhdr);
#endif

	/* 检查是否为可执行文件 */
	if(elfhdr->e_type != ET_EXEC) {
		fprintf(stderr, "%s: target isn't executable file\n", __func__);
		goto load_err;	
	}
	/* 检查当前架构是否匹配 */
	if(1 == 0 
#ifdef __mips__ 
	 || elfhdr->e_machine != EM_MIPS
#endif
	) {
		fprintf(stderr, "%s: unexpected target machine\n", __func__);
		goto load_err;	
	}
	/* 检查某些长度信息 */
	if(elfhdr->e_ehsize != sizeof(*elfhdr) 
				|| elfhdr->e_phentsize != sizeof(*ephdr)) {
		fprintf(stderr, "%s: unexpected header len\n", __func__);
		goto load_err;	
	}

	/* 获取程序头表 */
	if(!(ephdrtab = (struct elf64_phdr *)malloc(elfhdr->e_phnum * sizeof(*ephdr)))) {
		fprintf(stderr, "%s: cannot allocate memory for ELF program header table\n",
				 __func__);
		goto load_err;
	}
	/* 注意：这里如果文件过大导致e_phoff造成32位溢出时，将引起错误 */
	if(-1 == lseek(fd, (off_t)elfhdr->e_phoff, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		goto load_err;
	}
	if(read(fd, (uint8_t *)ephdrtab, elfhdr->e_phnum * sizeof(*ephdr)) 
				!= elfhdr->e_phnum * sizeof(*ephdr)) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		goto load_err;
	}
#if ELF_DEBUG
	for(i = 0; i < elfhdr->e_phnum; ++i)
		dump_elf64_phdr(ephdrtab + i);
#endif

	/* 从p_offset由低到高顺序依次加载程序段到内存  */
	while(1) {
		ephdr = NULL;
		lowest_off = ~0LL;
		/* 找到最小的可加载程序段 */
		for(i = 0; i < elfhdr->e_phnum; ++i) {
			if(PT_LOAD == ephdrtab[i].p_type) {
				if(ephdrtab[i].p_offset < lowest_off) {
					ephdr = ephdrtab + i;
					lowest_off = ephdrtab[i].p_offset;
				}
			}
		}
		if(!ephdr)			//遍历完所有需要加载到内存的程序段
			break;	
		if(ephdr->p_filesz) {
			/* 检查加载地址是否合法 */
			if(__elf64_chkloadaddr(ephdr)) {
				fprintf(stderr, "%s: load address[0x%llx/0x%llx] is unexpeced\n",
					__func__, ephdr->p_vaddr, ephdr->p_memsz);
				goto load_err;	
			}
			/* 将当前段拷贝到内存 */
			if(__elf64_loadseg2mem(fd, ephdr)) 
				goto load_err;	
			/* 清零p_filesz到p_memsz之间的区域 */	
			__elf64_clearseg(ephdr);
		}
		ephdr->p_type = PT_NULL;
	}

	/**
 	 *  注意：这里仅取64位地址的低32位，因此要求内核在编译的时候必须
 	 *  	  将入口地址设置为32位兼容模式 
 	 */
	*entry = (unsigned long)elfhdr->e_entry;

	free(ephdrtab);
	free(elfhdr);

	return EXEC_LOAD_RES_SUCCESS;

load_err:
	if(ephdrtab)
		free(ephdrtab);
	if(elfhdr)
		free(elfhdr);

	return EXEC_LOAD_RES_LOADERR;
}

static int __elf32_loader(int fd, int flags, unsigned long *entry)
{
	unsigned long i;
	Elf32_Off lowest_off;
	struct elf32_ehdr *elfhdr = NULL;
	struct elf32_phdr *ephdrtab = NULL, *ephdr;

	/* 获取ELF格式头 */
	if(!(elfhdr = (struct elf32_ehdr *)malloc(sizeof(*elfhdr)))) {
		fprintf(stderr, "%s: cannot allocate memory for ELF header\n", __func__);
		return EXEC_LOAD_RES_LOADERR;
	}
	if(-1 == lseek(fd, 0UL, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		goto load_err;
	}
	if(read(fd, (uint8_t *)elfhdr, sizeof(*elfhdr)) != sizeof(*elfhdr)) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		goto load_err;
	}
#if ELF_DEBUG
	dump_elf32_ehdr(elfhdr);
#endif

	/* 检查是否为可执行文件 */
	if(elfhdr->e_type != ET_EXEC) {
		fprintf(stderr, "%s: target isn't executable file\n", __func__);
		goto load_err;	
	}
	/* 检查当前架构是否匹配 */
	if(1 == 0 
#ifdef __mips__ 
	 || elfhdr->e_machine != EM_MIPS
#endif
	) {
		fprintf(stderr, "%s: unexpected target machine\n", __func__);
		goto load_err;	
	}
	/* 检查某些长度信息 */
	if(elfhdr->e_ehsize != sizeof(*elfhdr) 
				|| elfhdr->e_phentsize != sizeof(*ephdr)) {
		fprintf(stderr, "%s: unexpected header len\n", __func__);
		goto load_err;	
	}

	/* 获取程序头表 */
	if(!(ephdrtab = (struct elf32_phdr *)malloc(elfhdr->e_phnum * sizeof(*ephdr)))) {
		fprintf(stderr, "%s: cannot allocate memory for ELF program header table\n",
				 __func__);
		goto load_err;
	}
	/* 注意：这里如果文件过大导致e_phoff造成32位溢出时，将引起错误 */
	if(-1 == lseek(fd, elfhdr->e_phoff, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		goto load_err;
	}
	if(read(fd, (uint8_t *)ephdrtab, elfhdr->e_phnum * sizeof(*ephdr)) 
				!= elfhdr->e_phnum * sizeof(*ephdr)) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		goto load_err;
	}
#if ELF_DEBUG
	for(i = 0; i < elfhdr->e_phnum; ++i)
		dump_elf32_phdr(ephdrtab + i);
#endif

	/* 从p_offset由低到高顺序依次加载程序段到内存  */
	while(1) {
		ephdr = NULL;
		lowest_off = ~0L;
		/* 找到最小的可加载程序段 */
		for(i = 0; i < elfhdr->e_phnum; ++i) {
			if(PT_LOAD == ephdrtab[i].p_type) {
				if(ephdrtab[i].p_offset < lowest_off) {
					ephdr = ephdrtab + i;
					lowest_off = ephdrtab[i].p_offset;
				}
			}
		}
		if(!ephdr)			//遍历完所有需要加载到内存的程序段
			break;	
		if(ephdr->p_filesz) {
			/* 检查加载地址是否合法 */
			if(__elf32_chkloadaddr(ephdr)) {
				fprintf(stderr, "%s: load address[0x%x/0x%x] is unexpeced\n",
					__func__, ephdr->p_vaddr, ephdr->p_memsz);
				goto load_err;	
			}
			/* 将当前段拷贝到内存 */
			if(__elf32_loadseg2mem(fd, ephdr)) 
				goto load_err;	
			/* 清零p_filesz到p_memsz之间的区域 */	
			__elf32_clearseg(ephdr);
		}
		ephdr->p_type = PT_NULL;
	}

	/**
 	 *  注意：这里仅取64位地址的低32位，因此要求内核在编译的时候必须
 	 *  	  将入口地址设置为32位兼容模式 
 	 */
	*entry = elfhdr->e_entry;

	free(ephdrtab);
	free(elfhdr);

	return EXEC_LOAD_RES_SUCCESS;

load_err:
	if(ephdrtab)
		free(ephdrtab);
	if(elfhdr)
		free(elfhdr);

	return EXEC_LOAD_RES_LOADERR;
}

/**
 *  描述：加载指定文件描述符对应的可执行文件
 *  参数：fd，源文件描述符；flags，加载标志位；entry，返回可执行文件的程序入口地址
 *  返回：成功返回EXEC_LOAD_RES_SUCCESS；格式出错返回EXEC_LOAD_RES_DETFAIL；
 *  	  出错返回EXEC_LOAD_RES_LOADERR
 */
int elf_loader(int fd, int flags, unsigned long *entry)
{
	int ret;
	uint8_t elf_ident[EI_NIDENT];

	/* 检查ELF文件头标识信息 */	
	if(-1 == lseek(fd, 0UL, SEEK_SET)) {
		fprintf(stderr, "%s: seek failed(corrupt object file?), %s\n", 
					__func__, strerror(errno));
		return EXEC_LOAD_RES_BADFMT;
	}
	if(read(fd, elf_ident, sizeof(elf_ident)) != sizeof(elf_ident)) {
		fprintf(stderr, "%s: read failed(corrupt object file?), %s\n", 
					__func__, errno ? strerror(errno) : "uncomplete");
		return EXEC_LOAD_RES_LOADERR;
	}
	if(ELF_FMT_BADELF == (ret = __check_elf_ident(elf_ident))) {
		fprintf(stderr, "%s: bad ELF format\n", __func__);
		return EXEC_LOAD_RES_LOADERR;
	} else if(ELF_FMT_NOELF == ret)
		return EXEC_LOAD_RES_BADFMT;
	else if(ELF_FMT_ELF64 == ret)
		return __elf64_loader(fd, flags, entry);
	//else if(ELF_FMT_ELF32 == ret)
	
	return __elf32_loader(fd, flags, entry);
}

static struct exec_type elf_type = {
	.et_name = "elf",
	.et_loader = elf_loader,
	.et_type = EXEC_TYPE_ELF,
	.et_flags = EXEC_FLAGS_AUTODETECT,
};

static void __attribute__((constructor)) init_elf_type(void)
{
	exec_type_register(&elf_type);
}

