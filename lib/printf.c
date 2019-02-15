
/**
 * Copyright(c) 2016-8-24 Shangwen Wu 
 *
 * printf函数实现
 * 
 */
#include <stdio.h>			

int printf(const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);
	
	return n;
}
