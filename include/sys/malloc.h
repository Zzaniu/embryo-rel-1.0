

/**
 * Copyright(c) 2016-1-12 Shangwen Wu	
 *
 * 系统内存管理相关头文件
 * 
 */

#ifndef __SYS_MALLOC_H__
#define __SYS_MALLOC_H__

/* 每个cluster均包含一个kmemusage结构，该结构包含其所属的bucket索引以及当前已分配空间大小信息 */
struct kmemusage {
	short ku_index;							//指向bucket数组的索引
	union {
		ushort ncluster;					//当空间快大小大于MAXALLOCSAVE，用于表示当前内存快包含的cluster数
		ushort npieces;						//当空间块大小小于MAXALLOCSAVE，用于表示当前buckets的链表中有多少个allocsize小块节点数
	} ku_un;
};

#define vatokup(va)			(&kmemusage[((caddr_t)va - kmembase) >> PGSHIFT])		//将地址转换为对应的kmemusage指针

#define MINBUCKET			4							//最小bucket数组个数
#define MINALLOCSIZE		(1 << MINBUCKET)			//每次最小分配的字节数（16字节）

/**
 * 根据传入的需要的空间大小计算出bucket中的索引，分配空间的大小与bucket索引之间的关系为：size = 1 < index
 * index==4:(0, MINALLOCSIZE]; index==5:(MINALLOCSIZE, 2*MINALLOCSIZE]; 
 * index==6:(2*MINALLOCSIZE, 4*MINALLOCSIZE]; index==7:(4*MINALLOCSIZE, 8*MINALLOCSIZE]; 
 * index==8:(8*MINALLOCSIZE, 16*MINALLOCSIZE]; index==9(16*MINALLOCSIZE, 32*MINALLOCSIZE]
 * index==10:(32*MINALLOCSIZE, 64*MINALLOCSIZE]; index==11(64*MINALLOCSIZE, 128*MINALLOCSIZE]
 * index==12:(128*MINALLOCSIZE, 256*MINALLOCSIZE]; index==13(256*MINALLOCSIZE, 512*MINALLOCSIZE]
 * index==14:(512*MINALLOCSIZE, 1024*MINALLOCSIZE]; index==15(1024*MINALLOCSIZE, 2048*MINALLOCSIZE]
 * index==16:(2048*MINALLOCSIZE, 4096*MINALLOCSIZE]; index==17(4096*MINALLOCSIZE, 8192*MINALLOCSIZE]
 * index==18:(8192*MINALLOCSIZE, 16384*MINALLOCSIZE]; index==19(16384*MINALLOCSIZE, 32768*MINALLOCSIZE]
 */
#define BUCKETINDEX(size) (									\
		((size) <= 128 * MINALLOCSIZE) ?					\
			((size) <= 8 * MINALLOCSIZE) ?					\
				((size) <= 2 * MINALLOCSIZE) ?				\
					((size) <= 1 * MINALLOCSIZE) ?			\
						(MINBUCKET + 0) : (MINBUCKET + 1)	\
					:										\
					((size) <= 4  * MINALLOCSIZE) ?			\
						(MINBUCKET + 2) : (MINBUCKET + 3)	\
				:											\
				((size) <=  32 * MINALLOCSIZE) ?			\
					((size) <= 16 * MINALLOCSIZE) ?			\
						(MINBUCKET + 4) : (MINBUCKET + 5)	\
					:										\
					((size) <= 64  * MINALLOCSIZE) ?		\
						(MINBUCKET + 6) : (MINBUCKET + 7)	\
			:												\
			((size) <= 2048 * MINALLOCSIZE) ?				\
				((size) <= 512 * MINALLOCSIZE) ?			\
					((size) <= 256 * MINALLOCSIZE) ?		\
						(MINBUCKET + 8) : (MINBUCKET + 9)	\
					:										\
					((size) <= 1024  * MINALLOCSIZE) ?		\
						(MINBUCKET + 10) : (MINBUCKET + 11)	\
				:											\
				((size) <=  8192 * MINALLOCSIZE) ?			\
					((size) <= 4096 * MINALLOCSIZE) ?		\
						(MINBUCKET + 12) : (MINBUCKET + 13)	\
					:										\
					((size) <= 16384  * MINALLOCSIZE) ?		\
						(MINBUCKET + 14) : (MINBUCKET + 15)	\
		)

/* 包含一个cluster的链表以及当前bucket的使用信息，该结构将和freelist共同组成一个记录空闲空间的单链，该结构充当该链的头节点 */
struct kmembucket {
	caddr_t kb_next;									//指向下一个空闲空间位置，值为NULL表示当前链表内没有可用空间
	caddr_t kb_last;									//指向最后一个空闲空间的位置
};

/**
 * 用于管理内存空间的链表节点，该节点位于空闲空间的基地值，指向下一个节点的指针会在malloc调用后被调用者的数据覆盖，
 * 而在初始化和free中指向下一个空闲空间的freelist 
 */
struct freelist  {
	caddr_t next;
};

extern caddr_t kmembase;				//defined in kern_malloc.c
extern caddr_t kmemlimit;			

extern void kmeminit(int raw_memsize);
extern void *kmem_malloc(size_t size);
extern void *kmem_zmalloc(size_t size);
extern void *__kmem_alloc(size_t size);
extern void kmem_free(void *alloc);

#endif //__SYS_MALLOC_H__

