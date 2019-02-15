
/**
 * Copyright(c) 2019-1-17 Shangwen Wu	
 *
 * 加载可执行文件相关定义 
 * 
 */

#ifndef __LOAD_EXEC_H__
#define __LOAD_EXEC_H__

#include <sys/list.h>					

/* 可执行文件格式类型 */
#define EXEC_TYPE_UNKOWN	0
#define EXEC_TYPE_ELF		1				/* elf32 & elf64 file */
#define EXEC_TYPE_BIN		2				/* binary file */
#define EXEC_TYPE_TXT		3				/* text file */

#define EXEC_FLAGS_AUTODETECT	0x01		/* 该文件类型支持自动格式检测 */

#define EXEC_LOAD_RES_SUCCESS	0			/* 加载成功 */
#define EXEC_LOAD_RES_BADFMT	1			/* 非法文件格式 */
#define EXEC_LOAD_RES_LOADERR	2			/* 加载出错 */

/* 可执行文件格式类型结构定义 */
struct exec_type {
	char *et_name;
	int (*et_loader)(int, int, unsigned long *);
	int et_type;
	int et_flags;
	struct list_head et_list;
};

extern unsigned long load_exec(int fd, int type, int flags);		//defined load.c
extern int exec_type_register(struct exec_type *);
extern void exec_type_unregister(struct exec_type *);

#endif //__LOADER_H__
