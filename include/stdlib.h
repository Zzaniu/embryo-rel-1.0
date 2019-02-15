
/**
 * Copyright(c) 2016-8-17 Shangwen Wu	
 *
 * stdlib.h头文件
 * 
 */

#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <common.h>
#include <sys/types.h>

extern void *malloc(size_t nbytes);

extern void free(void *ap);

#endif //__STDLIB_H__
