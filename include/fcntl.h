
/**
 * Copyright(c) 2016-8-25 Shangwen Wu	
 *
 * 文件操作相关头文件
 * 
 */
#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_RDONLY		0x0001
#define O_WRONLY		0x0002
#define O_RDWR			0x0004
#define O_ACCMODE		0x0007

extern int open(const char *fname, int mode);
extern int ioctl(int fd, unsigned long cmd, ...);

#endif	//__FCNTL_H__

