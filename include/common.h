
/**
 * Copyright(c) 2015-6-6 Shangwen Wu	
 *
 * 常用定义
 * 
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define BIT_0			0x1
#define BIT_1			0x2
#define BIT_2			0x4
#define BIT_3			0x8
#define BIT_4			0x10
#define BIT_5			0x20
#define BIT_6			0x40
#define BIT_7			0x80
#define BIT_8			0x100
#define BIT_9			0x200
#define BIT_10			0x400
#define BIT_11			0x800
#define BIT_12			0x1000
#define BIT_13			0x2000
#define BIT_14			0x4000
#define BIT_15			0x8000
#define BIT_16			0x10000
#define BIT_17			0x20000
#define BIT_18			0x40000
#define BIT_19			0x80000
#define BIT_20			0x100000
#define BIT_21			0x200000
#define BIT_22			0x400000
#define BIT_23			0x800000
#define BIT_24			0x1000000
#define BIT_25			0x2000000
#define BIT_26			0x4000000
#define BIT_27			0x8000000
#define BIT_28			0x10000000
#define BIT_29			0x20000000
#define BIT_30			0x40000000
#define BIT_31			0x80000000

#define SZ_1K			0x00000400
#define SZ_4K			0x00001000
#define SZ_16K			0x00004000
#define SZ_64K			0x00010000
#define SZ_256K			0x00040000
#define SZ_1M			0x00100000
#define SZ_4M			0x00400000
#define SZ_16M			0x01000000

#define LOG_1K			10
#define LOG_4K			12
#define LOG_16K			14
#define LOG_64K			16
#define LOG_256K		18
#define LOG_1M			20
#define LOG_4M			22
#define LOG_16M			24

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NR
#define NR(arr)			(sizeof(arr) / sizeof(arr[0]))
#endif

#define min(a, b)		((a) < (b) ? (a) : (b))
#define max(a, b)		((a) > (b) ? (a) : (b))

#define ROUNDUP(x)		(1 + (((x) - 1) | (sizeof(long) - 1)))

#endif	/* __COMMON_H__ */
