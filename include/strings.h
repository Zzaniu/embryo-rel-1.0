
/**
 * Copyright(c) 2016-9-1 Shangwen Wu	
 *
 * strings库遗留函数头文件
 * 
 */

#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <common.h>
#include <sys/types.h>

extern void bzero(void *s, size_t n);
extern void bcopy(const void *s, void *d, size_t n);
extern size_t bcmp(const void *a1, const void *a2, size_t n);

#endif //__STRINGS_H___

