
/**
 * Copyright(c) 2016-11-18 Shangwen Wu	@wsw 26th years 
 *
 * 内存访问相关操作命令 
 * 
 */

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <command.h>
#include <fs/termio.h>

#define MORE_MAX_LINE			10
#define DEFAULT_DATASIZE		1

extern int get_rsa_reg(register_t *vp, const char *p);
#ifdef HAVE_QUAD
extern int get_rsa(u_quad_t *vp, const char *p);					//defined in rsa.c
#else
extern int get_rsa(unsigned long *vp, const char *p);
#endif
extern int getopt(int argc, const char *argv[], int *optind, \
		char **optval, const char *pattern);				//defined in getopt.c
extern uint8_t load_byte(unsigned long addr);				//defined in rsa.c
extern void store_byte(void *a, u_int8_t v);				//defined in rsa.c
extern void store_half(void *a, u_int16_t v);				//defined in rsa.c
extern void store_word(void *a, u_int32_t v);				//defined in rsa.c
extern void store_dword(void *a, u_int64_t v);				//defined in rsa.c

static unsigned long display_mem(char *prnbuf, unsigned long addr, int datasize);

/**
 * 描述：内存地址写命令，注意！该命令仅支持PMON的32位地址，对于64位地址暂不支持
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_modify(int argc, const char *argv[])
{
	int datasize = 0, optind;
	char opt, *optval = NULL;
	unsigned long addr;
	unsigned long long sz, val, qaddr;
	char prnbuf[100] = {0}, *p = NULL;	
	extern char line_buf[];					//defined in main.c

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "bhwd")) != EOF) {
		switch(opt) {
			case 'b':
				datasize |= 1;
			break;
			case 'h':
				datasize |= 2;
			break;
			case 'w':
				datasize |= 4;
			break;
			case 'd':
				datasize |= 8;
			break;
			default:							//BADOPT
				return -2;
		}
	}
	if(datasize & (datasize - 1)) {
		fprintf(stderr, "%s: multiple data types specified\n", *argv);
		return -2;
	}
	if(!datasize) 
		datasize = DEFAULT_DATASIZE;
	
	if(optind >= argc) {
		fprintf(stderr, "%s: need specify modify address\n", *argv);
		return -2;
	}
	
	if(get_rsa(&qaddr, argv[optind])) {
		fprintf(stderr, "%s: illegal modify address -- %s\n", *argv, argv[optind]);
		return -2;
	}
	addr = (unsigned long)qaddr;
	if(addr & (datasize - 1)) {
		fprintf(stderr, "%08x: unaligned address\n", addr);
		return -2;
	}

	if(++optind < argc) {				/* 命令模式 */
		for(; optind < argc; ++optind, addr += datasize) {
			if(get_rsa_reg(&val, argv[optind])) {
				fprintf(stderr, "%s: illegal value -- %s\n", *argv, argv[optind]);
				return -2;
			}
			switch (datasize) {
				case 1:
					store_byte((void *)addr, val);
				break;
				case 2:
					store_half((void *)addr, val);
				break;
				case 4:
					store_word((void *)addr, val);
				break;
				case 8:
					store_dword((void *)addr, val);
				break;
			}
		}
	} else {							/* 交互模式 */
		while(1) {
			switch (datasize) {
				case 1:
					val = *(u_int8_t *)addr;
				break;
				case 2:
					val = *(u_int16_t *)addr;
				break;
				case 4:
					val = *(u_int32_t *)addr;
				break;
				case 8:
					val = *(u_int64_t *)addr;
				break;
			}
			printf("0x%08x %0*llx ", addr, 2 * datasize, val);
			line_buf[0] = '\0';
			get_line(line_buf, 0);			
			for(p = line_buf; isspace(*p); ++p)
				;
			if('\0' == *p) 
				addr += datasize;
			else if('^' == *p || '-' == *p) 
				addr -= datasize;
			else if('.' == *p)
				break;
			else if('=' == *p)
				;					//重读一次
			else if(!get_rsa_reg(&val, p)) {
				switch (datasize) {
					case 1:
						store_byte((void *)addr, val);
					break;
					case 2:
						store_half((void *)addr, val);
					break;
					case 4:
						store_word((void *)addr, val);
					break;
					case 8:
						store_dword((void *)addr, val);
					break;
				}
				addr += datasize;
			} 
		}
	}

	return 0;
}

/**
 * 描述：内存地址读命令，注意！该命令仅支持PMON的32位地址，对于64位地址暂不支持
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_dump(int argc, const char *argv[])
{
	int datasize = 0, optind;
	char opt, *optval = NULL;
	unsigned long addr;
	unsigned long long ln, sz, qaddr;
	char prnbuf[100] = {0};	

	optind = 0;
	while((opt = getopt(argc, argv, &optind, &optval, "bhwd")) != EOF) {
		switch(opt) {
			case 'b':
				datasize |= 1;
			break;
			case 'h':
				datasize |= 2;
			break;
			case 'w':
				datasize |= 4;
			break;
			case 'd':
				datasize |= 8;
			break;
			default:							//BADOPT
				return -2;
		}
	}
	if(datasize & (datasize - 1)) {
		fprintf(stderr, "%s: multiple data types specified\n", *argv);
		return -2;
	}
	
	if(optind >= argc) {
		fprintf(stderr, "%s: need specify dump address\n", *argv);
		return -2;
	}
	
	if(get_rsa(&qaddr, argv[optind])) {
		fprintf(stderr, "%s: illegal dump address -- %s\n", *argv, argv[optind]);
		return -2;
	}
	addr = (unsigned long)qaddr;

	if(++optind < argc) {
		if(get_rsa(&ln, argv[optind])) {
			fprintf(stderr, "%s: illegal line count -- %s\n", *argv, argv[optind]);
			return -2;
		}
		
		sz = 0;
	} else {
		sz = ln = MORE_MAX_LINE;
	}

	if(!datasize) 
		datasize = DEFAULT_DATASIZE;

	ioctl(STDIN, TCSBREAK, NULL);
	while(1) {
		addr = display_mem(prnbuf, addr, datasize);
		if(more(prnbuf, &ln, sz))
			break;
	}

	return 0;
}

/**
 * 显示内存函数
 */
static unsigned long display_mem(char *prnbuf, unsigned long addr, int datasize)
{
	int i;
	uint8_t databuf[16] = {0};
	uint8_t *_addr = (uint8_t *)addr;

	for(i = 0; i < 16; ++i) {
		databuf[i] = load_byte(addr + i);
	}
	
	sprintf(prnbuf, "%08x  ", addr);
	prnbuf += strlen(prnbuf);
	for(i = 0; i < 16; i += datasize) {
		if(8 == i)
			strcat(prnbuf++, "  ");
		switch(datasize) {
			case 1:
				sprintf(prnbuf, "%02x ", _addr[i]);
			break;
			case 2:
				sprintf(prnbuf, "%04x ", *((uint16_t *)(_addr + i)));
			break;
			case 4:
				sprintf(prnbuf, "%08x ", *((uint32_t *)(_addr + i)));
			break;
			case 8:
				sprintf(prnbuf, "%016llx ", *((uint64_t *)(_addr + i)));
			break;
		}
		prnbuf += strlen(prnbuf);
	}
	strcat(prnbuf++, " ");
	for(i = 0; i < 16; ++i) {
		if(!isprint(databuf[i]))
			strcat(prnbuf, ".");
		else
			strichr(prnbuf + 1, databuf[i]);
	}

	return addr + 16;
}

static struct optdesc dump_opts[] = {
	{
		.name = "-b", 
		.desc = "display memory as bytes",
	},
	{
		.name = "-h",
		.desc = "display memory as half-words",
	},
	{
		.name = "-w", 
		.desc = "display memory as words",
	},
	{
		.name = "-d", 
		.desc = "display memory as double-words",
	},
	{
		.name = "addr", 
		.desc = "dump memory addrss",
	},
	{
		.name = "cnt", 
		.desc = "count of dump memory line",
	},
	{},
};

static struct optdesc modify_opts[] = {
	{
		.name = "-b", 
		.desc = "modify memory as bytes",
	},
	{
		.name = "-h",
		.desc = "modify memory as half-words",
	},
	{
		.name = "-w", 
		.desc = "modify memory as words",
	},
	{
		.name = "-d", 
		.desc = "modify memory as double-words",
	},
	{
		.name = "addr", 
		.desc = "modify memory addrss",
	},
	{
		.name = "<hexval>", 
		.desc = "value of need-modify memory",
	},
	{
		.name = "Interactive Options", 
		.desc = "",
	},
	{
		.name = ".", 
		.desc = "quit",
	},
	{
		.name = "Enter", 
		.desc = "forward one",
	},
	{
		.name = "-|^", 
		.desc = "back one",
	},
	{
		.name = "=", 
		.desc = "re-read",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Memory"}},			//表组名
	{
		{"m"},
		"modify memory",
		"[-bhwd] addr [hexval]",
		modify_opts, 
		cmd_modify,
		2,
		99,
		0, 
	},
	{
		{"d"},
		"display memory",
		"[-bhwd] addr [cnt]",
		dump_opts, 
		cmd_dump,
		2,
		4,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
