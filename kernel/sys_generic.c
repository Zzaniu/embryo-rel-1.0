
/**
 * Copyright(c) 2017-8-18 Shangwen Wu 
 *
 * 系统调用相关底层实现函数
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/syscallarg.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/**
 * 描述：ioctl底层实现入口函数，该函数为了确保正确行，参数要严格于linux下的实现
 * 参数：args，一个register_t类型的参数数组；retval，指向返回值存放位置
 * 返回：要求返回errno对应的枚举值
 */
int sys_ioctl(struct proc *p, void *args, register_t *retval)
{
	int err = 0, fd;
	struct file *fp;
	struct filedesc *fdesc = p->p_fd;
	void *arg;
	unsigned long cmd;
	caddr_t kbuf = NULL;
	ulong len;
	struct sys_ioctl_args /*{
	syscallarg(int) fd;
	syscallarg(unsigned long) cmd;
	syscallarg(void *) arg;
	} */ *ap = (struct sys_ioctl_args *)args;

	fd = SCARG(ap, fd);
	cmd = SCARG(ap, cmd);
	arg = SCARG(ap, arg);

	log(LOG_DEBUG, "ioctl, fd=%d, cmd=0x%x, data=0x%x\n", fd, cmd, arg);

	/* 参数检查 */
	if(fd >= fdesc->fd_nfiles || !(fp = fdesc->fd_ofiles[fd])) {
		err = EBADF;
		goto failed;
	}

	if((fp->f_flag & O_RDWR) != O_RDWR) {
		err = EPERM;
		goto failed;
	}

	if((_IOC_DIR(cmd) & _IOC_WRITE) && (NULL == arg)) {
		err = EINVAL;
		goto failed;
	}

	if((len = _IOC_LEN(cmd)) > NBPG) {
		err = EINVAL;
		goto failed;
	}

	if(NULL == (kbuf = kmem_zmalloc(len))) {
		err = ENOBUFS;
		goto failed;
	}

	if((_IOC_DIR(cmd) & _IOC_WRITE) && len)
		copyin(kbuf, (caddr_t)arg, len);
	
	if(err = (*fp->f_ops->fo_ioctl)(fp, cmd, kbuf, p)) 
		goto failed;

	if((_IOC_DIR(cmd) & _IOC_READ) && len)
		copyout((caddr_t)arg, kbuf, len);
		
done:
	if(kbuf != NULL)
		kmem_free(kbuf);

	return err;

failed:
	
	*retval = -1;

	goto done;
}
