

/**
 * Copyright(c) 2017-5-3 Shangwen Wu	
 *
 * 用户IO描述
 * 
 */

#ifndef __SYS_UIO_H__
#define __SYS_UIO_H__

/* 一个IO单元，每次传送数据可由多个这样的单沿组成 */
struct iovec {
	caddr_t iov_base;				//用户数据基地值
	size_t iov_len;					//用户数据长度
};

/* 数据读写标识 */
enum uio_rwflag {
	UIO_WRITE,
	UIO_READ
};

/* 数据为用户曾还是系统曾 */
enum uio_segflag {
	UIO_USERSPACE,
	UIO_SYSSPACE
};

/* 用户数据信息描述结构体 */
struct uio {
	struct iovec *uio_iov;			//iov单元基地值（必须保证每个iov单元地址连续）
	ulong uio_iovcnt;				//iov单元个数
	enum uio_rwflag uio_rw;			//读写IO标识
	enum uio_segflag uio_segflag;	//运行态
	struct proc *uio_proc;			//UIO所属进程
	size_t uio_resid;				//剩余未处理字节数
};

extern int uiomove(caddr_t cp, ulong len, struct uio *uio);//defined in kern_misc.c

#endif //__SYS_UIO_H__

