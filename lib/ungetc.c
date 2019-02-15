
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * ungetc函数实现
 * 
 */
#include <stdio.h>			

int ungetc(int c, FILE *stream)
{
	stream->ungetcflag = 1;
	stream->ungetchar = c;
	return c;
}
