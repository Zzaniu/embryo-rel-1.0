
/**
 * Copyright(c) 2017-8-18 Shangwen Wu 
 *
 * 系统调用相关底层实现函数
 * 
 */

#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/sockio.h>
#include <sys/fileio.h>
#include <fcntl.h>
#include <net/if.h>

static int soo_write(struct file *fp, struct uio *io)
{

	return 0;
}

static int soo_read(struct file *fp, struct uio *io)
{

	return 0;
}

/**
 * 描述：socket接口曾ioctl实现
 */
static int soo_ioctl(struct file *fp, unsigned long cmd, caddr_t arg, struct proc *p)
{
	struct socket *so = (struct socket *)fp->f_data;

	if(!so)
		return ENXIO;

	if(0/* suser(p)*/)
		return EPERM;

	switch(cmd) {
		case FIONREAD:
			*(ulong *)arg = so->soc_rcvsb.sb_cc;
			return 0;	
	};

	if(_IOC_TYPE(cmd) == 'i') 
		return if_ioctl(so, cmd, arg);
	else 
		return ENOTTY;

	return 0;
}

static int soo_close(struct file *fp, struct proc *p)
{
	int ret = 0;
	
	if(fp->f_data)
		ret = soclose((struct socket*)fp->f_data);
	fp->f_data = NULL;

	return ret;
}

struct fileops socketops = {
	.fo_write = soo_write,
	.fo_read = soo_read,
	.fo_ioctl = soo_ioctl,
	.fo_close = soo_close,
};

