
/**
 * Copyright(c) 2016-11-15 Shangwen Wu 
 *
 * sprintf函数实现
 * 
 */
#include <stdio.h>			

int sprintf(char *buf, const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = vsprintf(buf, fmt, ap);
	va_end(ap);
	
	return n;
}
