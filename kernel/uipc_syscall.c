
/**
 * Copyright(c) 2017-2-21 Shangwen Wu 
 *
 * 进程间通信相关底层实现函数，基本都是socket相关的删数 
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/syscallarg.h>
#include <sys/socketvar.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/syscall.h>
#include <mach/limits.h>
#include <fcntl.h>


/**
 * 描述：socket底层实现入口函数
 * 参数：args，一个register_t类型的参数数组；retval，指向返回值存放位置
 * 返回：要求返回errno对应的枚举值
 */
int sys_socket(struct proc *p, void *args, register_t *retval)
{
	int err = 0, fd;
	struct socket *sock = NULL;
	struct file *fp;
	extern struct fileops socketops;		//define in sys_scoket.c
	struct sys_socket_args /*{
	syscallarg(int) domain;
	syscallarg(int) type;
	syscallarg(int) protocol;
	} */ *ap = (struct sys_socket_args *)args;

	err = falloc(p, &fd, &fp);
	if(err) 
		goto failed;
	fp->f_type = FTYPE_SOCKET;
	fp->f_flag = O_RDWR;
	fp->f_ops = &socketops;
	
	err = socreate(&sock, SCARG(ap, domain), SCARG(ap, type), SCARG(ap, protocol)); 
	if(err) {
		ffree(p, fd);
		__open_err(fd);//释放用户太的fd资源
		goto failed;
	}
	
	fp->f_data = (caddr_t)sock;
	*retval = fd;

	return 0;

failed:
	*retval = -1;

	return err;
}

/**
 * 描述：bind底层实现入口函数
 * 参数：args，一个register_t类型的参数数组；retval，指向返回值存放位置
 * 返回：要求返回errno对应的枚举值
 */
int sys_bind(struct proc *p, void *args, register_t *retval)
{
	int err = 0;
	struct file *fp;
	struct mbuf *addr = NULL;
	struct sys_bind_args /*{
		syscallarg(int) sfd;
		syscallarg(const struct sockaddr *) laddr;
		syscallarg(socklen_t) laddrlen;
	}*/ *ap = (struct sys_bind_args *)args;

	if((err = getsock(&fp, p, SCARG(ap, sfd))) != 0)
		return err;
	if((err = sockargs(&addr, (caddr_t)SCARG(ap, laddr), SCARG(ap, laddrlen), MT_SONAME)) != 0)
		return err;
	
	err = sobind((struct socket *)fp->f_data, addr);

	mbuf_freem(addr);

	return err;
}

/**
 * 描述：sendto底层实现入口函数
 * 参数：args，一个register_t类型的参数数组；retval，指向返回值存放位置
 * 返回：要求返回errno对应的枚举值
 */
int sys_sendto(struct proc *p, void *args, register_t *retval)
{
	struct msghdr msg; 
	struct iovec iov;
	struct sys_sendto_args /*{
	syscallarg(int) sfd;
	syscallarg(const void *) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(const struct sockaddr *) to;
	syscallarg(socklen_t) tolen;
	} */ *ap = (struct sys_sendto_args *)args;

	msg.msg_soname = (caddr_t)SCARG(ap, to);
	msg.msg_sonamelen = SCARG(ap, tolen);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov.iov_base = (caddr_t)SCARG(ap, buf);
	iov.iov_len = SCARG(ap, len);
	msg.msg_iovcnt = 1;
	msg.msg_iov = &iov;

	return sendit(SCARG(ap, sfd), &msg, SCARG(ap, flags), retval, p);
}

/* sendto系统调用过渡函数 */
int sendit(int sfd, struct msghdr *msgp, int flags, register_t *retval, struct proc *p)
{
	ulong i, len;
	int err = 0;
	struct uio uio;
	struct file *fp;
	struct iovec *iov;
	struct mbuf *to = NULL, *control;

	uio.uio_iov = msgp->msg_iov;
	uio.uio_iovcnt = msgp->msg_iovcnt;
	uio.uio_rw = UIO_WRITE;
	uio.uio_segflag = UIO_USERSPACE;
	uio.uio_proc = p;
	uio.uio_resid = 0;

	if((err = getsock(&fp, p, sfd)) != 0)
		return err;
	
	for(i = 0, iov = uio.uio_iov; i < uio.uio_iovcnt; ++i, ++iov) 
		if(iov->iov_len > SIZE_MAX || (uio.uio_resid += iov->iov_len) > SIZE_MAX)
			return EINVAL;
	
	if(msgp->msg_soname) {
		if(err = sockargs(&to, msgp->msg_soname, msgp->msg_sonamelen, MT_SONAME))
			return err;
	} else 
		to = NULL;

	if(msgp->msg_control) {
		if(msgp->msg_controllen < sizeof(struct cmsghdr)) {
			err = EINVAL;
			goto out;
		}
		if(err = sockargs(&control, msgp->msg_control, msgp->msg_controllen, MT_CONTROL))
			goto out;
	} else 
		control = NULL;

	len = uio.uio_resid;
	/* 注意，soend函数将释放top占用的内存 */
	if(err = sosend((struct socket *)fp->f_data, to, &uio, NULL, control, flags)) {
		if(len != uio.uio_resid && (ERESTART == err || EINTR == err || EWOULDBLOCK == err))
			err = 0;
	}
	
	if(!err)
		*retval = len - uio.uio_resid;

out:
	if(to)
		mbuf_freem(to);

	return err;
}

/**
 * 描述：recvfrom底层实现入口函数
 * 参数：args，一个register_t类型的参数数组；retval，指向返回值存放位置
 * 返回：要求返回errno对应的枚举值
 */
int sys_recvfrom(struct proc *p, void *args, register_t *retval)
{
	int err;
	struct msghdr msg; 
	struct iovec iov;
	struct sys_recvfrom_args /*{
	syscallarg(int) sfd;
	syscallarg(void *) buf;
	syscallarg(size_t) len;
	syscallarg(int) flags;
	syscallarg(struct sockaddr *) from;
	syscallarg(socklen_t *) fromlen;
	} */ *ap = (struct sys_recvfrom_args *)args;

	if(SCARG(ap, fromlen)) {
		err = copyin((caddr_t)&msg.msg_sonamelen, (caddr_t)SCARG(ap, fromlen), sizeof(msg.msg_sonamelen));
		if(err)
			return err;
	} else 
		msg.msg_sonamelen = 0;

	msg.msg_soname = (caddr_t)SCARG(ap, from);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov.iov_base = (caddr_t)SCARG(ap, buf);
	iov.iov_len = SCARG(ap, len);
	msg.msg_iovcnt = 1;
	msg.msg_iov = &iov;
	msg.msg_flags = SCARG(ap, flags);

	return recvit(SCARG(ap, sfd), &msg, (caddr_t)SCARG(ap, fromlen), retval, p);
}

/* recvfrom系统调用过渡函数 */
int recvit(int sfd, struct msghdr *msgp, caddr_t namelenp, register_t *retval, struct proc *p)
{
	ulong i, len, clen, off;
	int err = 0;
	struct uio uio;
	struct file *fp;
	struct iovec *iov;
	struct mbuf *from = NULL, *control = NULL, *m;

	uio.uio_iov = msgp->msg_iov;
	uio.uio_iovcnt = msgp->msg_iovcnt;
	uio.uio_rw = UIO_READ;
	uio.uio_segflag = UIO_USERSPACE;
	uio.uio_proc = p;
	uio.uio_resid = 0;

	if((err = getsock(&fp, p, sfd)) != 0)
		return err;
	
	for(i = 0, iov = uio.uio_iov; i < uio.uio_iovcnt; ++i, ++iov) 
		if(iov->iov_len > SIZE_MAX || (uio.uio_resid += iov->iov_len) > SIZE_MAX)
			return EINVAL;

	len = uio.uio_resid;
	/* 注意，soreceive函数将分配from以及control的内存空间 */
	if(err = soreceive((struct socket *)fp->f_data, &from, &uio, 
		NULL, msgp->msg_control ? &control : NULL, &msgp->msg_flags)) {
		if(len != uio.uio_resid && (ERESTART == err || EINTR == err || EWOULDBLOCK == err))
			err = 0;
	}
	if(err)
		goto out;
	
	*retval = len - uio.uio_resid;
	
	if(msgp->msg_soname) {
		len = msgp->msg_sonamelen;
		if(len && from) {
			if(len > from->m_len)
				len = from->m_len;
			if(err = copyout(msgp->msg_soname, mtod(from, caddr_t), len))
				goto out;
		} else
			len = 0; 
		msgp->msg_sonamelen = len;
		if(namelenp && 
			(err = copyout(namelenp, (caddr_t)&len, sizeof(socklen_t))))
			goto out;
	}

	/* 这里针对PMON的问题进行改进，对control信息的处理涉及整个mbuf链，而非单个mbuf节点 */
	if(msgp->msg_control) {
		off = 0;
		m = control;
		clen = msgp->msg_controllen;
		while(clen) {
			if(NULL == m) {
				msgp->msg_flags |= MSG_CTRUNC;
				break;
			}
			len = min(clen, m->m_len);
			if(err = copyout(msgp->msg_control + off, mtod(m, caddr_t), len)) {
				msgp->msg_flags |= MSG_CTRUNC;
				msgp->msg_controllen = off;
				goto out;
			}
			off += len;
			m = m->m_next;
			clen -= len;
		}
		msgp->msg_controllen = off;
	}

out:
	if(from)
		mbuf_freem(from);
	if(control)
		mbuf_freem(control);

	return err;
}

/* 获取socket文件结构 */
int getsock(struct file **fpp, struct proc *p, int fd)
{
	struct file *fp;

	if(fd < 0 || fd >= NFILE || !(fp = p->p_fd->fd_ofiles[fd]))
		return EBADF;
	if(!(fp->f_type & FTYPE_SOCKET))
		return ENOTSOCK;
	
	*fpp = fp;

	return 0;
}

/* 将buf中数据拷贝到mbuf空间 */
int sockargs(struct mbuf **mp, caddr_t buf, ulong len, ushort type)
{
	struct mbuf *m;
	struct sockaddr *sa;

	if(len > MLEN)
		return EINVAL;
	
	if(!(m = mbuf_get(type)))
		return ENOBUFS;
	m->m_len = len;
	
	copyin(mtod(m, caddr_t), buf, len);
		
	*mp = m;

	if(MT_SONAME == type) {
		sa = (struct sockaddr *)buf;
		sa->sa_len = len;
	}

	return 0;
}
