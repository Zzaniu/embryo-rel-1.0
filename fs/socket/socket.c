
/**
 * Copyright(c) 2017-4-20 Shangwen Wu	
 *
 * socket设备相关
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <stdarg.h>
#include <fs/file.h>

//defined in kern_syscall.c
extern int soc_ioctl(int, unsigned long, void *arg); 
extern int soc_close(int fd);

/**
 * 描述：socket设备打开函数
 * 返回：打开成功返回当前文件描述符，否则返回-1
 */
static int socket_open(int fd, const char *filepath, int flags, int perms)
{
	//do nothing
	return fd;
}

static int socket_close(int fd)
{
	int ret;
	
	ret = soc_close(fd);

	__file[fd].data = NULL;

	return ret;
}

static ssize_t socket_read(int fd, void *buf, size_t len)
{
	return 0;
}

static ssize_t socket_write(int fd, const void *buf, size_t len)
{
	return 0;
}

static off_t socket_lseek(int fd, off_t off, int whence)
{
	return 0;
}

static int socket_ioctl(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	return soc_ioctl(fd, cmd, arg);
}

static struct file_system socket_fs = { 
	.fs_name = "socket",
	.fs_type = FS_SOCKET,
	.open = socket_open,
	.close = socket_close,
	.read = socket_read,
	.write = socket_write,
	.lseek = socket_lseek,
	.ioctl = socket_ioctl,
};

/**
 * 描述：终端设备文件模块初始化函数，注意由于该函数处于ctor段，因此该
 * 		函数将在__init函数中被调用
 */
static void __attribute__((constructor)) socketfs_init(void)
{
	fs_register(&socket_fs);
}
