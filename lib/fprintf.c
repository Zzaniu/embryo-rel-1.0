
/**
 * Copyright(c) 2016-11-18 Shangwen Wu 
 *
 * fprintf函数实现
 * 
 */
#include <stdio.h>			

int fprintf(FILE *fp, const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = vfprintf(fp, fmt, ap);
	va_end(ap);
	
	return n;
}
