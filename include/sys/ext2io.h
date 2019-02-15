

/**
 * Copyright(c) 2018-12-30 Shangwen Wu	
 *
 * ext2 fs ioctl相关定义 
 * 
 */

#ifndef __SYS_EXT2IO_H__
#define __SYS_EXT2IO_H__

#include <sys/ioctl.h>

struct stat;
struct dirent_info;

#define EIOCGFSTAT	_IOWR('e', 0, struct stat) 	//读取文件的inode信息
#define EIOCGDIRENT	_IOWR('e', 1, struct dirent_info) 	//读取文件的目录信息

#endif //__SYS_EXT2IO_H__

