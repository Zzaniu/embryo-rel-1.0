
/**
 * Copyright(c) 2016-8-17 Shangwen Wu	
 *
 * string.h头文件
 * 
 */

#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>
#include <sys/types.h>

extern char *strchr(const char *s, int c);
extern char *strcpy(char *d, const char *s);
extern char *strncpy(char *d, const char *s, size_t n);
extern void *memcpy(void *d, const void *s, size_t n);
extern size_t strlen(const char *s);
extern char *strcat(char *d, const char *s);
extern char *strncat(char *d, const char *s, size_t n);
extern int strcmp(const char *d, const char *s);
extern int strncmp(const char *d, const char *s, size_t n);
extern void *memset(void *s, int c, size_t n);
extern char *strstr(const char *s, const char *d);
/* 非标准字符串操作 */
extern char *strlchr(const char *s, int c);
extern char *strdchr(char *p);
extern char *strichr(char *p, int c);
extern char *stristr(char *p, const char *s);
extern int strempty(const char *s);
extern int strprefix(const char *d, const char *s);
extern void memcpy32(unsigned int *d, unsigned int *s, unsigned int n);
extern void memset32(unsigned int *s, int c, size_t n);
extern int strpat(const char *s0, const char *d0);

#endif	//__STRING_H__
