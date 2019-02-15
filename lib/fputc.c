
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * fputc函数实现
 * 
 */
#include <stdio.h>			
#include <unistd.h>			

int putc(int c, FILE *stream)
{
	return fputc(c, stream);
}

int fputc(int c, FILE *stream)
{
	if(1 == write(stream->fd, &c, 1))
		return c;
	else 
		return EOF;
}
