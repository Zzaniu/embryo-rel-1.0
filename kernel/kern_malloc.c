
/**
 * Copyright(c) 2016-1-12 Shangwen Wu 
 *
 * 系统级内存管理函数实现
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <stdio.h>
#include <strings.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <asm/cpu.h>
#include <mach/intr.h>

/* 系统内存相关参数，这些参数将通过initmips传入的raw_memsize被初始化 */
u_int64_t memsize_total;				//系统物理内存大小
u_int64_t memsize_high;					//系统256MB以上高端内存大小	
u_int64_t memsize_avail; 				//BIOS系统可用内存大小
caddr_t kmembase = NULL;				//系统可用内存基址
caddr_t kmemlimit = NULL;				//系统可用内存基址上界
static off_t kmemoffs = 0;				//系统可用内存偏移
static struct kmemusage *kmemusage;
static struct kmembucket buckets[MINBUCKET + 16 + 1] = {0};	//保存0～16*32768字节大小的空闲空间链表，用额外的一个位置表示大空间的bucket索引

/**
 * 描述：系统空间分配函数，注意：该函数只能分配以页大小为单元的空间
 */
void *__kmem_alloc(size_t size)
{
	char *alloc;

	size = round_page(size);
	if(size + kmemoffs > KMEM_SIZE) {
		log(LOG_WARNING, "__kmem_alloc: request for %lu bytes fails\n", size);
		return NULL;
	}
	
	alloc = kmembase + kmemoffs;
	kmemoffs += size;

	log(LOG_DEBUG, "__kmem_alloc: request for %lu bytes, base 0x%x\n", size, alloc);

	return (void *)alloc;
}

/**
 * 描述：释放系统资源，注意，当前系统不支持内存free
 */
void __kmem_free(void *alloc)
{
	panic("__kmem_free unsupport");
}

/**
 * 描述：系统级别的malloc函数实现，注意：该函数分配出来的大小为2的指数
 * 		 该函数只能管理大小小于MAXALLOCSAVE字节的空闲空间，对于大于MAXALLOCSAVE仅在kmemusage结构中保存部分使用信息，
 * 		 每次调用__kmem_alloc函数进行实际的空间分配时，都是以cluster为单位进行分配的，该函数对某些参数的值要求比较严
 * 		 格，比如allocsize的值必须要被cluster的大小整除，一旦不满足上述条件，将会导致地址访问越界
 */
void *kmem_malloc(size_t size)
{
	int s;
	short index = 0;
	size_t allocsize, cbytes;
	caddr_t va = NULL;
	struct kmemusage *pku;
	struct kmembucket *pkb;
	struct freelist *freep;

	s = splimp();
	if(size <= MAXALLOCSAVE) {
		index = BUCKETINDEX(size);
		pkb = &buckets[index];

		if(NULL == pkb->kb_next) {						//buckets链表中暂时没有已分配的节点
			allocsize = 1 << index;
			cbytes = round_cluster(allocsize);

			if(NULL == (va = __kmem_alloc(cbytes))) { 
				log(LOG_ERR, "kmem_alloc: out of memory\n");
				splx(s);
				return NULL;
			}
			pku = vatokup(va);
			pku->ku_index = index;
			pku->ku_un.npieces = 1;

			pkb->kb_next = va;							//每当新分配了新的空间，buckets中的记录将丢失
			while(1) {
				if(va + allocsize >= pkb->kb_next + cbytes)	//注意：这里的allocsize和cbytes必须要保证整除关系
					break;
				freep = (struct freelist *)va;
				va += allocsize;
				freep->next = va;
				++pku->ku_un.npieces;
			}
			((struct freelist *)va)->next = NULL;
			pkb->kb_last = va;
		}	
		va = pkb->kb_next;
		pkb->kb_next = ((struct freelist *)va)->next;
	} else {
		cbytes = round_cluster(size);					//对于大于MAXALLOCSAVE字节的空间以cluster为单位字节划分空间
		if(cbytes > KMEM_SIZE || NULL == (va = __kmem_alloc(cbytes))) { 
			log(LOG_ERR, "kmem_alloc: out of memory\n");
			splx(s);
			return NULL;
		}
		pku = vatokup(va);
		pku->ku_index = NR(buckets) - 1;				//最后一个索引表示大空间的索引
		pku->ku_un.ncluster = btoc(cbytes);
	}
	
	splx(s);
	return (void *)va;
}

/**
 * 描述：系统级别的malloc函数实现，该函数将清零当前分配的空间
 */
void *kmem_zmalloc(size_t size)
{
	void *p = kmem_malloc(size);
	
	if(p)
		bzero((caddr_t)p, size);

	return p;
}

/**
 * 描述，释放不用的空间，
 */
void kmem_free(void *va)
{
	int s;
	struct kmemusage *pku;
	struct kmembucket *pkb;
	struct freelist *freep;

	pku = vatokup(va);
		
	s = splimp();
	if(pku->ku_index < MINBUCKET || pku->ku_index >= NR(buckets)) 		//要释放的地址不在buckets的管理范围之中
		goto quit;
	
	if(pku->ku_index < NR(buckets) - 1)	{				//大小小于MAXALLOCSAVE字节的空间
		pkb = &buckets[pku->ku_index];
		if(NULL == pkb->kb_next) 
			pkb->kb_next = (caddr_t)va;
		else
			((struct freelist *)pkb->kb_last)->next = (caddr_t)va;
		((struct freelist *)va)->next = NULL;			//插入队尾
		pkb->kb_last = (caddr_t)va;
	} else {											//大空间不参与回收管理，直接释放
		pku->ku_index = 0;								//最后一个索引表示大空间的索引
		pku->ku_un.ncluster = 0;
		__kmem_free(va);
	}

quit:
	splx(s);
}

/**
 * 以cluster为单位给每个快分配一个kmemusage结构
 */
static void __kmeminit(void)
{
	unsigned long ncl;

	ncl = KMEM_SIZE / CLBYTES;

	if(!(kmemusage = (struct kmemusage *)__kmem_alloc(ncl * sizeof(struct kmemusage)))) {
		panic("out of memory for kmemusage");
	}
}

/**
 * 描述：初始化系统内存空间
 */
void kmeminit(int raw_memsize)
{
	u_int32_t base;								//bad codes 我们系统仅支持32位地址空间

	memsize_total = (raw_memsize & 0xff) << 29;
	memsize_high = memsize_total > 0x10000000 ? memsize_total - 0x10000000 : 0;
	memsize_avail = (memsize_total > 0x10000000 ? 0x10000000 : memsize_total) - 0x1000000;	//系统内存可用大小为低256MB减去代码占用的16MB空间
	if(memsize_avail < KMEM_SIZE * 2)		//总的可用空间必须为KMEM_SIZE的两倍
		panic("not enough memory for system");
	base = (memsize_avail - KMEM_SIZE) & ~PAGE_SIZE;	//页对齐

#ifdef __mips__ 
	if((u_int32_t)&kmembase < CPU_KSEG_UNCACHED) 
		kmembase = (caddr_t)PHY_TO_CACHED(base);
	else
		kmembase = (caddr_t)PHY_TO_UNCACHED(base);
#else
	kmembase = (caddr_t)base;	
#endif
	kmemlimit = kmembase + KMEM_SIZE;

	log(LOG_DEBUG, "meminfo: memsize_total=0x%llx, memsize_high=0x%llx, memsize_avail=0x%llx, kmembase=0x%x\n",
				memsize_total, memsize_high, memsize_avail, (u_int32_t)kmembase);

	/* 分配一段空间用于保存内存使用信息 */
	__kmeminit();
}

