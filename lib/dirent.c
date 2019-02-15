
/**
 * Copyright(c) 2018-12-01 Shangwen Wu	
 *
 * 目录文件操作相关函数
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fs/file.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/direntinfo.h>
#include <sys/ext2io.h>
#include <fs/file.h>

DIR *opendir(const char *path)
{
	DIR *dp;
	struct stat st;

	/* 文件名必须以/dev/fs/打头 */
	if(!strprefix(path, FSIO_PREFIX)) {
		errno = ENOENT;
		fprintf(stderr, "%s:  file's prefix is not \"/dev/fs\"\n", path);
		return NULL;
	}

	/* 检查文件是否为目录 */
	if(-1 == stat(path, &st)) 
		return NULL;
	
	if(!S_ISDIR(st.st_mode)) {	/* 非目录文件 */
		errno = ENOTDIR;
		return NULL;
	}

	if(!(dp = (DIR *)malloc(sizeof(DIR)))) {
		errno = ENOMEM;
		return NULL;
	}
	dp->fd = open(path, O_RDONLY); 
	dp->idx = 0;

	return dp;
}

struct dirent *readdir(DIR *dp)
{
	struct dirent_info info = {0};

	if(!dp) {
		errno = EINVAL;
		return NULL;
	}
	info.dir_idx = dp->idx;
	info.dir_name = dp->dir.d_name;

	if(-1 == ioctl(dp->fd, EIOCGDIRENT, &info)) 
		return NULL;

	if(info.dir_idx == dp->idx) {
		errno = 0;				//到达目录末尾
		return NULL;
	}

	++dp->idx;	//指向下一个目录项

	return &dp->dir;
}

int closedir(DIR *dp)
{
	int ret;

	ret = close(dp->fd);
	free(dp);
	
	return ret;
}

