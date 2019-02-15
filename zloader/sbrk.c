
/**
 * Copyright(c) 2016-1-16 Shangwen Wu 
 *
 * 内存索取函数实现
 * 
 */

#ifndef NULL
#define NULL ((void *)0)
#endif

/* 注意：zloader可用内存空间的起始地址为kseg0空间（带缓存）低256M内存的高4M字节 */
static char *membase = (char *)(0xffffffff80000000 + ((256 - 4) << 20));

/**
 * 描述：内存地址分割索取，将未加入内存管理的地址空间加入到空闲内存管理链表
 * 参数：size，需要分割的字节数
 * 返回：没有可用的空间时返回NULL
 */
char *sbrk(unsigned size)
{
	char *p = NULL;

	if(!(membase + size > (char *)(0xffffffff80000000 + (256 << 20)))) {
		p = membase;
		membase += size;
	}
	return p;
}
