
/**
 * Copyright(c) 2018-11-30 Shangwen Wu 
 *
 * stat系统接口实现
 * 
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fs/file.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ext2io.h>
#include <fs/file.h>

int stat(const char *path, struct stat *st)
{
	int fd;

	/* 文件名必须以/dev/fs/打头 */
	if(!strprefix(path, FSIO_PREFIX)) {
		errno = ENOENT;
		fprintf(stderr, "%s:  file's prefix is not \"/dev/fs\"\n", path);
		return -1;
	}

	if(-1 == (fd = open(path, O_RDONLY))) 
		return -1;

	if(-1 == ioctl(fd, EIOCGFSTAT, st)) 
		return -1;

	close(fd);

	return 0;
}

int fstat(int fd, struct stat *st)
{
	errno = EOPNOTSUPP;
	return -1;
}

int lstat(const char *path, struct stat *st)
{
	errno = EOPNOTSUPP;
	return -1;
}
