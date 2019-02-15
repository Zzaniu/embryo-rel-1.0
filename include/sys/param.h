

/**
 * Copyright(c) 2017-1-11 Shangwen Wu	
 *
 * 内存管理相关头文件
 * 
 */

#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#include <mach/param.h>

#define CLBYTES				(CLSIZE * PAGE_SIZE)
#define CLOFFSET			(CLBYTES - 1)
#define CLSHIFT				(PGSHIFT + CLLOG2)

#define round_cluster(size)	(((size) + CLOFFSET) & ~CLOFFSET)		//向上取clsuster大小

#if CLSIZE==1
	#define clrond(npg)			(npg)								//向上取clsuster页大小
#else
	#define clrond(npg)			(((npg) + (CLSIZE - 1)) & ~(CLSIZE - 1))
#endif

#define btoc(nb)			(((nb) + CLOFFSET) >> CLSHIFT)			//字节数转换成cluster数
#define ctob(nc)			((nc) << CLSHIFT)						//cluster数转换成字节数

/* 当于该值时，将不使用"1<<bucket数组索引"作为当前分配空间大小，而是根据实际需要大小的cluster向上取整值作为分配大小 */
#define MAXALLOCSAVE		(MINALLOCSIZE * 32 * 1024)

/* mbuf空间相关参数 */
#define MCLSHIFT			11
#define MCLBYTES			(1 << MCLSHIFT)							//mcluster大小为2KB

#define MAXMCLSIZE			256										//当前BIOS最大分配的mcluster数为256个

#endif //__SYS_PARAM_H__

