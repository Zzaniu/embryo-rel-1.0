
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * putchar函数实现
 * 
 */
#include <stdio.h>			

int putchar(int c)
{
	return fputc(c, stdout);
}
