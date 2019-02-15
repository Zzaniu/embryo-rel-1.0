

/**
 * Copyright(c) 2017-8-18 Shangwen Wu	
 *
 * 定义了ioctl命令的组织结构 
 * 
 */

#ifndef __SYS_IOCTL_H__
#define __SYS_IOCTL_H__

#define _IOC_DIRBITS	2
#define _IOC_LENBITS	14
#define _IOC_TYPEBITS	8
#define _IOC_NRBITS		8

#define _IOC_NONE		0x0
#define _IOC_READ		0x1
#define _IOC_WRITE		0x2

#define _IOC_DIRMASK	((1 << _IOC_DIRBITS) - 1)
#define _IOC_LENMASK	((1 << _IOC_LENBITS) - 1)
#define _IOC_TYPEMASK	((1 << _IOC_TYPEBITS) - 1)
#define _IOC_NRMASK		((1 << _IOC_NRBITS) - 1)

#define _IOC_NRSHIFT	(0)
#define _IOC_TYPESHIFT	(_IOC_NRBITS + _IOC_NRSHIFT)
#define _IOC_LENSHIFT	(_IOC_TYPEBITS + _IOC_TYPESHIFT)
#define _IOC_DIRSHIFT	(_IOC_LENBITS + _IOC_LENSHIFT)

#define _IOC(dir, len, type, nr)	( \
	((dir) & _IOC_DIRMASK) << _IOC_DIRSHIFT | \
	((len) & _IOC_LENMASK) << _IOC_LENSHIFT | \
	((type) & _IOC_TYPEMASK) << _IOC_TYPESHIFT | \
	((nr) & _IOC_NRMASK) << _IOC_NRSHIFT \
)

#define _IOC_SIZECHECKED(datatype)	(sizeof(datatype) < (1 << _IOC_LENBITS) ? sizeof(datatype) : 0)
#define _IO(type, nr)				_IOC(_IOC_NONE, 0, type, nr)
#define _IOW(type, nr, datatype)	_IOC(_IOC_WRITE, _IOC_SIZECHECKED(datatype), type, nr)
#define _IOR(type, nr, datatype)	_IOC(_IOC_READ, _IOC_SIZECHECKED(datatype), type, nr)
#define _IOWR(type, nr, datatype)	_IOC(_IOC_READ|_IOC_WRITE, _IOC_SIZECHECKED(datatype), type, nr)

#define _IOC_DIR(cmd)				(((cmd) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
#define _IOC_LEN(cmd)				(((cmd) >> _IOC_LENSHIFT) & _IOC_LENMASK)
#define _IOC_TYPE(cmd)				(((cmd) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#define _IOC_NR(cmd)				(((cmd) >> _IOC_NRSHIFT) & _IOC_NRMASK)

#endif //__SYS_IOCTL_H__

