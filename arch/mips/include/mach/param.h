
/**
 * Copyright(c) 2017-1-11 Shangwen Wu	
 *
 * 板级内存管理相关头文件
 * 
 */

#ifndef __MACH_PARAM_H__
#define __MACH_PARAM_H__

#define PAGE_SIZE			4096				//页大小
#define PAGE_MASK			0xfff				//页掩码
#define NBPG				4096
#define PGOFFSET			(NBPG - 1)
#define PGSHIFT				12
#define CLSIZE				1					//一个簇包含的页数
#define CLLOG2				0

#define round_page(size)	(((size) + PAGE_MASK) & ~PAGE_MASK)

#define NKMEMCLUSTERS		(64 * 1024 * 1024 / CLBYTES)

#define KMEM_SIZE			(NKMEMCLUSTERS * CLBYTES)						//内核系统可用内存空间大小

#endif //__MACH_PARAM_H__

