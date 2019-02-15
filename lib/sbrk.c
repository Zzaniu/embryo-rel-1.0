
/**
 * Copyright(c) 2016-1-16 Shangwen Wu 
 *
 * 内存索取函数实现
 * 
 */

#include <autoconf.h>
#include <stdlib.h>

#define ALLOCSIZE	(32 * 1024)					//这里要求libc的malloc函数可分配的最大大小为32KB

#if FIX_HEAP_SPACE == 0
static unsigned char allocbuf[ALLOCSIZE]; 
static unsigned char *allocp = allocbuf;
#endif

/**
 * 描述：内存地址分割索取，将未加入内存管理的地址空间加入到空闲内存管理链表
 * 参数：size，需要分割的字节数
 * 返回：没有可用的空间时返回NULL，注意由于PMON是32位的，因此这里返回指针时将会丢失高32位
 */
char *sbrk(size_t size)
{
#if FIX_HEAP_SPACE
	extern char *md_sbrk(size_t size);		//arch depend 
	
	return md_sbrk(size);
#else
	char *p = NULL;

	if(allocp + size <= allocbuf + ALLOCSIZE) {
		p = allocp;					//bad code
		allocp += size;
	}
	
	return p;
#endif
}
