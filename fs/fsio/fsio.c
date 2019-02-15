
/**
 * Copyright(c) 2018-11-27 Shangwen Wu	
 *
 * 文件系统IO相关
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <fs/file.h>
#include <fs/fsio.h>

static LIST_HEAD(fsio_list);

static int fsio_open(int fd, const char *filepath, int flags, int perms)
{
	int ret, found = 0;
	struct fsio_file *ff;
	struct fsio *fio;
	char *fsname, *dfname, *cp;
	char fs[FS_NAME_MAX];

	/* 解析文件系统名称与文件路径名 */
	if(!strprefix(filepath, FSIO_PREFIX)) {
		errno = EINVAL;	
		return -1;
	}
	fsname = (char *)(filepath + strlen(FSIO_PREFIX));
	for(cp = fsname; *cp && *cp != '@'; ++cp) 
		;
	if(!*cp) {
		/* 未指定文件系统名称 */
		fprintf(stderr, "missing filesystem name\n");
		errno = EINVAL;	
		return -1;
	}
	if((cp - fsname) >= FS_NAME_MAX) {
		/* 文件系统名称过长 */
		fprintf(stderr, "filesystem'name too long, max %d\n", FS_NAME_MAX);
		errno = ENAMETOOLONG;
		return -1;
	}
	strncpy(fs, fsname, cp - fsname);
	fs[cp - fsname] = 0;	
	dfname = ++cp;

	list_for_each_entry(fio, &fsio_list, list) {	
		if(!strcmp(fio->fsname, fs)) {
			if(!(ff = (struct fsio_file *)malloc(sizeof(struct fsio_file)))) {
				errno = ENOMEM;
				return -1;
			}
			ff->fio = fio;
			__file[fd].data = ff;
			found = 1;
			break;
		}
	}

	if(!found) {
		errno = ENOENT;
		return -1;
	}

	if(fio->ops->fio_open) {
		if((ret = fio->ops->fio_open(ff, dfname, flags)) != 0) {
			free(__file[fd].data);
			__file[fd].data = NULL;
			return ret;
		}
	}

	return fd;
}

static int fsio_release(int fd)
{
	int ret = 0;

	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;

	if(ff->fio->ops->fio_close) 
		 ret = ff->fio->ops->fio_close(ff);

	free(__file[fd].data);
	__file[fd].data = NULL;

	return ret;
}

static ssize_t fsio_read(int fd, void *buf, size_t len)
{
	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;

	if(!ff->fio->ops->fio_read) {
		errno = EINVAL;
		return -1;
	}
	
	return ff->fio->ops->fio_read(ff, buf, len);
}

static ssize_t fsio_write(int fd, const void *buf, size_t len)
{
	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;

	if(!ff->fio->ops->fio_write) {
		errno = EINVAL;
		return -1;
	}

	return ff->fio->ops->fio_write(ff, buf, len);
}

static off_t fsio_lseek(int fd, off_t off, int whence)
{
	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;

	if(!ff->fio->ops->fio_lseek) {
		errno = EINVAL;
		return -1;
	}

	return ff->fio->ops->fio_lseek(ff, off, whence);
}

static loff_t fsio_llseek(int fd, loff_t off, int whence)
{
	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;

	if(!ff->fio->ops->fio_llseek) {
		errno = EINVAL;
		return -1;
	}

	return ff->fio->ops->fio_llseek(ff, off, whence);
}

static int fsio_ioctl(int fd, unsigned long cmd, ...)
{
	struct fsio_file *ff = (struct fsio_file *)__file[fd].data;
	va_list ap;
	void *arg;

	if(!ff->fio->ops->fio_ioctl) {
		errno = EINVAL;
		return -1;
	}

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	return ff->fio->ops->fio_ioctl(ff, cmd, arg);
}

int fsio_register(struct fsio *fio)
{
	list_add_tail(&fio->list, &fsio_list);
	
	return 0;
}

void fsio_unregister(struct fsio *fio)
{
	list_del(&fio->list);
}

static struct file_system fsio_fs = { 
	.fs_name = "fs",
	.fs_type = FS_FSIO,
	.open = fsio_open,
	.close = fsio_release,
	.read = fsio_read,
	.write = fsio_write,
	.lseek = fsio_lseek,
	.lseek64 = fsio_llseek,
	.ioctl = fsio_ioctl,
};

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) fsio_init(void)
{
	fs_register(&fsio_fs);
}
