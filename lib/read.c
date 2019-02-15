
/**
 * Copyright(c) 2016-8-22 Shangwen Wu 
 *
 * read系统接口实现
 * 
 */
#include <stdio.h>
#include <sys/types.h>
#include <fs/file.h>
#include <errno.h>
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count)
{
	if(fd >= OPEN_MAX || !__file[fd].valid) {
		errno = EBADF;
		return -1;
	}
	
	if(__file[fd].fs->read) 
		return (*__file[fd].fs->read)(fd, buf, count);
	else {
		errno = EOPNOTSUPP;
		return -1;
	}
}

