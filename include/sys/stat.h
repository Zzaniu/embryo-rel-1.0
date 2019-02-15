

/**
 * Copyright(c) 2018-11-30 Shangwen Wu	
 *
 * stat相关头文件
 * 
 */

#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__

#include <sys/types.h>

/* file type & mode @linux2.6.32 */
#define S_IFMT  	00170000
#define S_IFSOCK 	0140000
#define S_IFLNK	 	0120000
#define S_IFREG  	0100000
#define S_IFBLK  	0060000
#define S_IFDIR  	0040000
#define S_IFCHR  	0020000
#define S_IFIFO  	0010000
#define S_ISUID  	0004000
#define S_ISGID  	0002000
#define S_ISVTX  	0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU 	00700
#define S_IRUSR 	00400
#define S_IWUSR 	00200
#define S_IXUSR 	00100

#define S_IRWXG 	00070
#define S_IRGRP 	00040
#define S_IWGRP 	00020
#define S_IXGRP 	00010

#define S_IRWXO 	00007
#define S_IROTH 	00004
#define S_IWOTH 	00002
#define S_IXOTH 	00001

#define S_IRWXUGO	(S_IRWXU|S_IRWXG|S_IRWXO)
#define S_IALLUGO	(S_ISUID|S_ISGID|S_ISVTX|S_IRWXUGO)
#define S_IRUGO		(S_IRUSR|S_IRGRP|S_IROTH)
#define S_IWUGO		(S_IWUSR|S_IWGRP|S_IWOTH)
#define S_IXUGO		(S_IXUSR|S_IXGRP|S_IXOTH)

/* stat结构体定义，主要用于描述文件的inode信息 */
struct stat {
	mode_t st_mode;					/* 文件类型以及访问权限 */
	ino_t st_ino;					/* inode号 */
	dev_t st_dev;					/* 设备号 */
	dev_t st_rdev;					/* 特殊文件设备号 */
	nlink_t st_nlink;				/* 硬链接数 */
	uid_t st_uid;					/* 所有者uid */
	gid_t st_gid;					/* 所有者gid */
	off_t st_size;					/* 一般文件数据内容大小 */
	time_t st_atime;				/* 最后访问时间 */
	time_t st_mtime;				/* 最后修改时间 */
	time_t st_ctime;				/* 文件最后状态改变时间 */
	blkcnt_t st_blksize;			/* 最优IO的块大小 */
	blksize_t st_blocks;			/* 共有多少个数据块 */
};

#endif //__SYS_STAT_H__

