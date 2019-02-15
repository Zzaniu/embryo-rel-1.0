
/**
 * Copyright(c) 2016-8-24 Shangwen Wu 
 *
 * fputs函数实现
 * 
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int fputs(const char *s, FILE *fp)
{
	return write(fp->fd, s, strlen(s));
}
