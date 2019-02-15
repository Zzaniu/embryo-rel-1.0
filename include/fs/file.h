
/**
 * Copyright(c) 2016-8-17 Shangwen Wu	
 *
 * 文件接口相关头文件
 * 
 */
#ifndef __FS_FILE_H__
#define __FS_FILE_H__

#include <linklist.h>

/* 文件操作接口定义部分 */

#define FS_NULL			0
#define FS_TTY			1
#define FS_SOCKET		2
#define FS_NETIO		3
#define FS_DEVIO		4
#define FS_FSIO			5

typedef struct file_system { 
	unsigned char *fs_name;
	int fs_type;
	int (*open)(int , const char *, int, int);
	int (*close)(int);
	ssize_t (*read)(int, void *, size_t);
	ssize_t (*write)(int, const void *, size_t);
	off_t (*lseek)(int, off_t, int);
	loff_t (*lseek64)(int, loff_t, int);
	int (*ioctl)(int, unsigned long, ...);
	SLIST_ENTRY(file_system) fs_next;
}File_System;

extern void fs_register(struct file_system *);
extern struct file_system *fs_first(void);
extern struct file_system *fs_end(void);
extern struct file_system *fs_next(struct file_system *cur);

/* 文件描述符定义部分 */

typedef struct File {
	int valid;
	void *data;						//一般用于保存FD和物理设备索引之间对应的映射关系
	struct file_system *fs;
} File;

extern File __file[];				//保存所有设备文件操作接口，文件描述符fd就是该数组的索引


/* 物理设备在BIOS中的文件名 */
#define DEVICE_PREFIX			"/dev/"
#define DEVIO_CHAR_DEVICE		"/dev/char/"
#define DEVIO_BLOCK_DEVICE		"/dev/block/"
#define FSIO_PREFIX				"/dev/fs/"
#define TTY_DEVICE				"tty"
#define TTY_SERIAL_DEVICE		"/dev/tty0"			//串口IO设备
#define TTY_STDIO_DEVICE		"/dev/tty1"			//基本输入输出设备（显示器+键盘）

#endif //__FS_FILE_H__
