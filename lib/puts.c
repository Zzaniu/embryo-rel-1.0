
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * puts函数实现
 * 
 */
#include <stdio.h>			

int puts(const char *s)
{
	return fputs(s, stdout);
}
