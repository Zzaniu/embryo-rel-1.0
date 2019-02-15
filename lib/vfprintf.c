
/**
 * Copyright(c) 2016-8-24 Shangwen Wu 
 *
 * vfprintf函数实现
 * 
 */
#include <stdio.h>			

#define VFPRINTF_BUF_SIZE			1024

int vprintf(const char *fmt, va_list ap)
{
	return vfprintf(stdout, fmt, ap);
}

int vfprintf(FILE *fp, const char *fmt, va_list ap)
{
	char buf[VFPRINTF_BUF_SIZE];

	vsprintf(buf, fmt, ap);

	return fputs(buf, fp);
}
