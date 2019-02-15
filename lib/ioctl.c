
/**
 * Copyright(c) 2016-8-25 Shangwen Wu 
 *
 * ioctl系统接口实现
 * 
 */
#include <common.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <fs/file.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int ioctl(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg = NULL;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	if(fd >= OPEN_MAX || !__file[fd].valid) {
		errno = EBADF;
		return -1;
	}
	
	if(__file[fd].fs->ioctl) 
		return (*__file[fd].fs->ioctl)(fd, cmd, arg);
	else {
		errno = EOPNOTSUPP;
		return -1;
 	}
}

