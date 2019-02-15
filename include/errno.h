
/**
 * Copyright(c) 2017-2-21 Shangwen Wu	
 *
 * errno相关头文件
 * 
 */

#ifndef __ERRNO_H__
#define __ERRNO_H__

#include <sys/errno.h>

extern void perror(const char *s);
extern char *strerror(int errnum);

#endif //__ERRNO_H__
