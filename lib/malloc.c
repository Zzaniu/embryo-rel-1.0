
/**
 * Copyright(c) 2016-1-16 Shangwen Wu 
 *
 * malloc-free内存管理函数实现
 * 
 */

typedef double ALIGN;

#include <stdlib.h>

#define NALLOC 128					//每次索取内存地址空间的最小HEADER块长度

/**
 * 描述：header为每个分配空间的头节点，ptr指向下一个空闲空间的header，
 * 		 size为当前空闲空间大小
 */
typedef union header {
	struct {
		union header *ptr;		//指向空闲空间链表的下一个节点
		unsigned size;			//当前节点的空间可用字节长度，包括HEADER节点本身占用的长度
	}s;
	ALIGN x;					//注意：每次分配空间将以ALIGN（8字节）对齐
}HEADER;

static HEADER base;				//空闲链表头节点
static HEADER *allocp = NULL;	//指向当前空闲新分配（释放）节点的上一个节点，此节点的ptr即为下一次将要分配的空闲空间指针

extern char *sbrk(size_t size);			//define in sbrk.c
void *coremore(size_t nunits);
void free(void *ap);

/**
 * 描述：以8字节对齐分配内存空间
 * 参数：nbytes, 需要分配空间大小
 * 返回：成功返回指向分配空间数据首地址，分配失败返回NULL
 */
void *malloc(size_t nbytes)
{
	/* 计算需要几个HEADER节点大小，+1是因为除了保存数据还需要另外保存头节点 */	
	size_t nunits = (nbytes + sizeof(HEADER) - 1) / sizeof(HEADER) + 1;
	HEADER *q, *p;				//q,p为链表遍历的前后指针

	if(NULL == (q = allocp)) {		//空闲表头节点初始化
		base.s.ptr = q = allocp = &base;		//空表的ptr指向自己
		base.s.size = 0;
	}

	/* 遍历空闲链表，直到找到所需大小空间或到达末尾并索取更多内存 */
	for(p = q->s.ptr; ; q = p, p = q->s.ptr) {
		if(p->s.size >= nunits) {
			if(p->s.size == nunits) 	//大小刚好
				q->s.ptr = p->s.ptr;
			else {						//大于所需空间时，将截取当前节点的后面部分，并记录新分配节点大小
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			allocp = q;					//将空闲表当前位置指向刚分配节点的前一节点
			return ((char *)(p + 1));	//返回数据空间的起始地址
		}
		if(p == allocp) 				//到达链表末尾
			if(NULL == (p = coremore(nunits))) 
				return NULL;			//分配失败
	}		
}

/**
 * 描述：分配更多存储区到空闲列表中，128*8字节对齐（最小快）
 * 参数：需要的HEADER个数
 * 返回：失败返回NULL，成功返回指向新分配节点的前一节点（allocp）
 */
void *coremore(size_t nunits)
{
	char *p = NULL;
	HEADER *ap;
	/* 计算需要多少个128*HEADER字节块 */
	size_t nalloc = ((nunits + NALLOC - 1) / NALLOC) * NALLOC;
	if(NULL == (p = sbrk(nalloc * sizeof(HEADER)))) 
		return NULL;
	ap = (HEADER *)p;
	ap->s.size = nalloc;
	free((char *)(ap + 1));
	return allocp;
}

/**
 * 描述：内存释放函数，回收当前数据指针对应节点的空间，并将该节点插入到空闲链表中
 * 参数：释放的数据指针（注意，HEADER对应调用者是不可见的，因此调用者只能传递数据区开始的部分）
 * 注意：此函数对于重复释放一个空间，将会导致死循环
 */
void free(void *ap)
{
	HEADER *q, *p;
	p = (HEADER *)ap - 1;

	for(q = allocp; !(p > q && p < q->s.ptr); q = q->s.ptr) 
		//比较难得理解的地方，此处考虑了&base与实际存储区的位置关系，同时又兼顾了链表为空的情况	
		if(q->s.ptr <= q && (p > q || p < q->s.ptr))	
			break;
		/* 在这里可以考虑处理重复释放的问题 */
		/* if(p == q)... */
	/* 找到插入链表位置位置 */
	if(p + p->s.size == q->s.ptr) {	//可合并后半部分
		p->s.ptr = q->s.ptr->s.ptr;
		p->s.size += q->s.ptr->s.size;
	} else
		p->s.ptr = q->s.ptr;
	if(q + q->s.size == p) {		//可合并前半部分
		q->s.ptr = p->s.ptr;
		q->s.size += p->s.size;
	} else
		q->s.ptr = p;	
	allocp = q;
}



