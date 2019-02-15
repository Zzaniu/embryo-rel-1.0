
/**
 * Copyright(c) 2018-8-22 Shangwen Wu	
 *
 * mips架构依赖DMA相关代码 
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/malloc.h>
#include <asm/cpu.h>
#include <asm/cache.h>
#include <asm/dma-mapping.h>

/**
 * 描述：虚拟地址到DMA地址转换
 */
static inline dma_addr_t __dma_map(void *vaddr)
{
	/* 注意：当vaddr位于kseg0空间，那么其物理地址刚好与DMA地址空间是一一对应的 */
	return (dma_addr_t)VA_TO_PHY(vaddr);
}

/**
 * 描述：DMA地址到kseg0虚拟地址转换
 */
static inline void *__dma_unmap(dma_addr_t addr)
{
	return (void *)PHY_TO_VA(addr);
}

/**
 * 描述：分配一段内存空间，该空间保证DMA与CPU访问的一致性
 */
void *dma_alloc_coherent(void *dev, size_t size, dma_addr_t *dma_handler)
{
	void *buf_va;

	if(NULL == (buf_va = kmem_zmalloc(size))) 
		return NULL;
	
	/* 
     * 注意：buf_va分配出来的内存位于kseg0区域，因为bios仅使用了低256M内存
 	 *	 	 的一部分，而该部分内存被映射到kseg0中，下面的操作用于将
 	 */
	cpu_flush_cache_io((vm_offset_t)buf_va, (vm_size_t)size, SYNC_W);
	/* 将虚拟地址映射为DMA可以访问的物理地址 */
	*dma_handler = __dma_map(buf_va);
	/* 返回uncache空间地址，以保证DMA与CPU访问数据的一致性 */
	buf_va = (void *)CACHED_TO_UNCACHED(buf_va);

	return buf_va;
}

/**
 *描述：释放一致性空间
 */
void dma_free_coherent(void *dev, size_t size, void *vaddr, dma_addr_t dma_handler)
{
	kmem_free((void *)UNCACHED_TO_CACHED(vaddr));
}

/**
 * 描述：将一段空间映射为DMA地址
 * 注意：ptr参数指向的地址只能位于kseg0区域
 */
dma_addr_t dma_map_single(void *dev, void *ptr, size_t size, enum dma_direction dir)
{
	cpu_flush_cache_io((vm_offset_t)ptr, (vm_size_t)size, dir);

	return __dma_map(ptr);
}

/**
 * 描述：将一段空间取消映射
 * 注意：ptr参数指向的地址只能位于kseg0区域
 */
void dma_unmap_single(void *dev, dma_addr_t addr, size_t size, enum dma_direction dir)
{
	void *ptr;

	ptr = __dma_unmap(addr);
	cpu_flush_cache_io((vm_offset_t)ptr, (vm_size_t)size, dir);
}


