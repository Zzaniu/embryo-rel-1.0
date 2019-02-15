
/**
 * Copyright(c) 2018-12-01 Shangwen Wu	
 *
 * 目录文件操作相关头文件
 * 
 */
#ifndef __DIRENT_H__
#define __DIRENT_H__

#define NAME_MAX	255

struct dirent {
	char d_name[NAME_MAX + 1];			//NAME_MAX必须与文件系统中设置的大小限制一致
};

typedef struct DIR_s {
	struct dirent dir;
	ulong idx;				/* 当前目录项索引 */
	int fd;
} DIR;

extern DIR *opendir(const char *path);
extern struct dirent *readdir(DIR *dp);
extern int closedir(DIR *dp);

#endif	//__DIRENT_H___

