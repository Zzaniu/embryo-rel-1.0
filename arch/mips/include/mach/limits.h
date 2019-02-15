
/**
 * Copyright(c) 2017-5-3 @PMON @Shangwen Wu	
 *
 * 板级相关数据类型上下界 
 * 
 */
#ifndef __MACH_LIMITS_H__
#define __MACH_LIMITS_H__

#define UCHAR_MAX	255
#define CHAR_MAX	127
#define CHAR_MIN	(-128)

#define USHRT_MAX	65535
#define SHRT_MAX	32767
#define SHRT_MIN	(-32768)

#define UINT_MAX	0xffffffff
#define INT_MAX		2147483647
#define INT_MIN		(-2147483647 - 1)

#define ULONG_MAX	0xffffffff
#define LONG_MAX	2147483647
#define LONG_MIN	(-2147483647 - 1)

#define SSIZE_MAX	INT_MAX
#define SIZE_MAX	UINT_MAX

#endif
