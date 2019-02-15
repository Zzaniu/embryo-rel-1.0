

/**
 * Copyright(c) 2016-8-17 Shangwen Wu	
 *
 * 类型相关头文件
 * 
 */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <mach/types.h>

/* 大小定义 */
typedef unsigned long int size_t;
typedef long int ssize_t;
typedef long int off_t;
typedef off_t off32_t;
typedef long long int loff_t;
typedef loff_t off64_t;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef u8 u_int8_t;
typedef u16 u_int16_t;
typedef u32 u_int32_t;
typedef u64 u_int64_t;

typedef char * caddr_t;

typedef u_int32_t pid_t;

typedef uint32_t n_time;

typedef u_int8_t sa_family_t;

typedef u_int32_t socklen_t;

typedef u32	time_t;

/* struct stat */
typedef u16	mode_t;
typedef u32 dev_t;
typedef u32	ino_t;
typedef u16	uid_t;
typedef u16	gid_t;
typedef u16	nlink_t;
typedef u32	blkcnt_t;
typedef u32	blksize_t;

#endif //__SYS_TYPES_H__

