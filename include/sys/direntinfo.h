
/**
 * Copyright(c) 2018-12-01 Shangwen Wu	
 *
 * 目录项操作相关头文件
 * 
 */
#ifndef __SYS_DIRENTINFO_H__
#define __SYS_DIRENTINFO_H__

#define NAME_MAX	255

struct dirent_info {
	ulong dir_idx;
	char *dir_name;			/* 该指针需要调用者提供存储空间 */
};

#endif	//__SYS_DIRENTINFO_H__

