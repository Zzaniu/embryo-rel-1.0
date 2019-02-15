
/**
 * Copyright(c) 2016-8-22 Shangwen Wu 
 *
 * unistd.h头文件实现
 * 
 */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#include <sys/types.h>

/* lseek */
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

struct stat;

extern loff_t llseek(int fd, loff_t off, int whence);
extern off_t lseek(int fd, off_t off, int whence);
extern ssize_t write(int fd, const void *buf, size_t count);
extern ssize_t read(int fd, void *buf, size_t count);
extern int close(int fd);
extern int stat(const char *path, struct stat *st);
extern int fstat(int fd, struct stat *st);
extern int lstat(const char *path, struct stat *st);

#endif	//__UNISTD_H__
