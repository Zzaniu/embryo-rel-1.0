
/**
 * Copyright(c) 2018-10-09 Shangwen Wu	
 *
 *  FILE文件相关IOCTL命令
 */

#ifndef __SYS_FILEIO_H__
#define __SYS_FILEIO_H__

#include <sys/ioctl.h>

#define FIONREAD	_IOR('f', 1, ulong)	

#endif //__SYS_FILEIO_H__

