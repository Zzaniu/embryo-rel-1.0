
/**
 * Copyright(c) 2016-3-7 Shangwen Wu	
 *
 * 环境变量相关头文件
 * 
 */
#ifndef __ENV_H__
#define __ENV_H__

#define NVAR	64				//当前BIOS最大支持64个环境变量

typedef struct envpair {
	char *name;
	char *val;
} envpair_t;

extern int envinit(void);		//defined in env.c

#endif	//__ENV_H__

