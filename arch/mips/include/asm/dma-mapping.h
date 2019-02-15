
/**
 * Copyright(c) 2018-8-22 Shangwen Wu	
 *
 * MIPS DMA操作定义
 * 
 */

#ifndef __ASM_DMA_MAPPING_H__
#define	__ASM_DMA_MAPPING_H__

enum dma_direction {
	DMA_FROM_DEVICE = SYNC_R,
	DMA_TO_DEVICE = SYNC_W,
	DMA_NONE = 2,
};

extern void *dma_alloc_coherent(void *dev, size_t size, dma_addr_t *dma_handler);//defined dma.c
extern void dma_free_coherent(void *dev, size_t size, void *vaddr, dma_addr_t dma_handler);
extern dma_addr_t dma_map_single(void *dev, void *ptr, size_t size, enum dma_direction dir);
extern void dma_unmap_single(void *dev, dma_addr_t addr, size_t size, enum dma_direction dir);

#endif /* __ASM_DMA_MAPPING_H__ */

