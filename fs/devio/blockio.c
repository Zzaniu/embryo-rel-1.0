
/**
 * Copyright(c) 2018-11-12 Shangwen Wu
 * 设备文件支持（FS层，分字符设备和块设备类型）
 * bad code 该模块将应用层与内核层之间耦合在一起，属于PMON遗传的槽糕设计
 */
#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <fs/blkfile.h>
#include <fs/file.h>

static int blkio_open(int fd, const char *filepath, int flags, int perms)
{
	struct block_file *bf;
	char *dname, *cp;

	if(!strprefix(filepath, DEVIO_BLOCK_DEVICE)) {
		errno = EINVAL;	
		return -1;
	}

	if(!(bf = (struct block_file *)malloc(sizeof(struct block_file)))) {
		errno = ENOMEM;	
		return -1;
	}

	dname = (char *)(filepath + strlen(DEVIO_BLOCK_DEVICE));
	for(cp = dname; *cp; ++cp) 
		if('@' == *cp || '/' == *cp)
			break;
	/* 下面两个字段必须在调用下层open函数之前初始化好 */
	strncpy(bf->bf_name, dname, cp - dname);	//注意：这里是否有必要检查dname长度
	bf->bf_name[cp - dname] = 0;	
	bf->bf_ops = &blkops;
	__file[fd].data = bf;
	
	if(bf->bf_ops->bfo_open) {
		if((errno = bf->bf_ops->bfo_open(bf, flags)) != 0) {
			free(__file[fd].data);
			__file[fd].data = NULL;
			return -1;
		}
	} else {
		free(__file[fd].data);
		errno = EINVAL;
		return -1;
	}

	return fd;
}

static int blkio_release(int fd)
{
	struct block_file *bf = (struct block_file *)__file[fd].data;

	if(bf->bf_ops->bfo_close) 
		errno = bf->bf_ops->bfo_close(bf);

	free(__file[fd].data);
	__file[fd].data = NULL;

	return errno ? -1 : 0;
}

static ssize_t blkio_read(int fd, void *buf, size_t len)
{
	struct block_file *bf = (struct block_file *)__file[fd].data;

	if(!bf->bf_ops->bfo_read) {
		errno = EINVAL;
		return -1;
	}
	
	return bf->bf_ops->bfo_read(bf, buf, len);
}

static ssize_t blkio_write(int fd, const void *buf, size_t len)
{
	struct block_file *bf = (struct block_file *)__file[fd].data;

	if(!bf->bf_ops->bfo_write) {
		errno = EINVAL;
		return -1;
	}
	
	return bf->bf_ops->bfo_write(bf, buf, len);
}

static loff_t blkio_lseek(int fd, loff_t off, int whence)
{
	struct block_file *bf = (struct block_file *)__file[fd].data;

	if(!bf->bf_ops->bfo_lseek) {
		errno = EINVAL;
		return -1;
	}

	return bf->bf_ops->bfo_lseek(bf, off, whence);
}

static int blkio_ioctl(int fd, unsigned long cmd, ...)
{
	struct block_file *bf = (struct block_file *)__file[fd].data;
	va_list ap;
	void *arg;

	if(!bf->bf_ops->bfo_ioctl) {
		errno = EINVAL;
		return -1;
	}

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	if(errno = bf->bf_ops->bfo_ioctl(bf, cmd, arg))
		return -1;

	return 0;
}

static struct file_system blkio_fs = { 
	.fs_name = "block",
	.fs_type = FS_DEVIO,
	.open = blkio_open,
	.close = blkio_release,
	.read = blkio_read,
	.write = blkio_write,
	.lseek64 = blkio_lseek,
	.ioctl = blkio_ioctl,
};

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) blkio_fs_init(void)
{
	fs_register(&blkio_fs);
}
