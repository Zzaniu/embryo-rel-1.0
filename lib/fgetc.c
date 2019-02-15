
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * fgetc函数实现
 * 
 */
#include <stdio.h>			
#include <unistd.h>			

int fgetc(FILE *stream)
{
	return getc(stream);
}

int getc(FILE *stream)
{
	char c;

	if(stream->ungetcflag) {
		stream->ungetcflag = 0;
		return stream->ungetchar;
	}

	if(1 == read(stream->fd, &c, 1))
		return c;
	else
		return EOF;
}

