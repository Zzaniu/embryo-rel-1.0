
/**
 * Copyright(c) 2018-11-12 Shangwen Wu 
 *
 * lseek系统接口实现
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

off_t lseek(int fd, off_t off, int whence)
{
	if(fd >= OPEN_MAX || !__file[fd].valid) {
		errno = EBADF;
		return -1;
	}
	
	if(__file[fd].fs->lseek) 
		return (*__file[fd].fs->lseek)(fd, off, whence);
	else {
		errno = EOPNOTSUPP;
		return -1;
 	}
}

loff_t llseek(int fd, loff_t off, int whence)
{
	if(fd >= OPEN_MAX || !__file[fd].valid) {
		errno = EBADF;
		return -1;
	}
	
	if(__file[fd].fs->lseek64) 
		return (*__file[fd].fs->lseek64)(fd, off, whence);
	else {
		errno = EOPNOTSUPP;
		return -1;
 	}
}
