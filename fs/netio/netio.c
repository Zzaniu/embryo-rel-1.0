
/**
 * Copyright(c) 2018-9-6 Shangwen Wu	
 *
 * net网络文件访问相关
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/list.h>
#include <string.h>
#include <stdlib.h>
#include <url.h>
#include <stdarg.h>
#include <errno.h>
#include <fs/file.h>
#include <fs/netio.h>

static LIST_HEAD(netio_list);

static int netio_open(int fd, const char *filepath, int flags, int perms)
{
	int ret, found = 0;
	struct URL url;
	struct netio_file *nf;
	struct netio *nio;

	if(url_parse(filepath, &url)) {
		errno = EINVAL;
		return -1;
	}
	
	list_for_each_entry(nio, &netio_list, list) {	
		if(!strcmp(nio->proto, url.protocol)) {
			if(!(nf = (struct netio_file *)malloc(sizeof(struct netio_file)))) {
				errno = ENOMEM;
				return -1;
			}
			nf->nio = nio;
			__file[fd].data = nf;
			found = 1;
			break;
		}
	}

	if(!found) {
		errno = ENOENT;
		return -1;
	}

	if(nio->ops->nio_open) {
		if((ret = nio->ops->nio_open(nf, &url, flags)) != 0) {
			free(__file[fd].data);
			__file[fd].data = NULL;
			return ret;
		}
	}

	return fd;
}

static int netio_release(int fd)
{
	int ret = 0;

	struct netio_file *nf = (struct netio_file *)__file[fd].data;

	if(nf->nio->ops->nio_close) 
		 ret = nf->nio->ops->nio_close(nf);

	free(__file[fd].data);
	__file[fd].data = NULL;

	return ret;
}

static ssize_t netio_read(int fd, void *buf, size_t len)
{
	struct netio_file *nf = (struct netio_file *)__file[fd].data;

	if(!nf->nio->ops->nio_read) {
		errno = EINVAL;
		return -1;
	}
	
	return nf->nio->ops->nio_read(nf, buf, len);
}

static ssize_t netio_write(int fd, const void *buf, size_t len)
{
	struct netio_file *nf = (struct netio_file *)__file[fd].data;

	if(!nf->nio->ops->nio_write) {
		errno = EINVAL;
		return -1;
	}

	return nf->nio->ops->nio_write(nf, buf, len);
}

static off_t netio_lseek(int fd, off_t off, int whence)
{
	struct netio_file *nf = (struct netio_file *)__file[fd].data;

	if(!nf->nio->ops->nio_lseek) {
		errno = EINVAL;
		return -1;
	}

	return nf->nio->ops->nio_lseek(nf, off, whence);
}

static int netio_ioctl(int fd, unsigned long cmd, ...)
{
	struct netio_file *nf = (struct netio_file *)__file[fd].data;
	va_list ap;
	void *arg;

	if(!nf->nio->ops->nio_ioctl) {
		errno = EINVAL;
		return -1;
	}

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	return nf->nio->ops->nio_ioctl(nf, cmd, arg);
}

int netio_register(struct netio *nio)
{
	list_add_tail(&nio->list, &netio_list);
	
	return 0;
}

void netio_unregister(struct netio *nio)
{
	list_del(&nio->list);
}

static struct file_system netio_fs = { 
	.fs_name = "netio",
	.fs_type = FS_NETIO,
	.open = netio_open,
	.close = netio_release,
	.read = netio_read,
	.write = netio_write,
	.lseek = netio_lseek,
	.ioctl = netio_ioctl,
};

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) netiofs_init(void)
{
	fs_register(&netio_fs);
}
