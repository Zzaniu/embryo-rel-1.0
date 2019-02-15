

/**
 * Copyright(c) 2018-11-14 Shangwen Wu	
 *
 * block ioctl相关定义 
 * 
 */

#ifndef __SYS_BLKIO_H__
#define __SYS_BLKIO_H__

#include <sys/ioctl.h>

struct mbr_partition;
struct partition_info;

#define BIOCGMBRPART	_IOWR('b', 0, struct mbr_partition) 	//读取磁盘的MBR分区信息
#define BIOCGPARTINF	_IOWR('b', 1, struct partition_info) 	//读取磁盘分区信息

#endif //__SYS_BLKIO_H__

