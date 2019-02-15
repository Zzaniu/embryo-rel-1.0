
/**
 * Copyright(c) 2015-10-13 Shangwen Wu	
 *
 * MIPS架构内存管理TLB寄存器及相关定义定义
 * 
 */

#ifndef __TLB_H__
#define __TLB_H__

/* CP0_PAGEMASK寄存器定义，该寄存器实现可配置的页大小 */
#define PAGE_MASK_4KB				0x00000000		/* 页大小4KB */
#define PAGE_MASK_16KB				0x00006000
#define PAGE_MASK_64KB				0x0001e000
#define PAGE_MASK_256KB				0x0007e000
#define PAGE_MASK_1MB				0x001fe000
#define PAGE_MASK_4MB				0x007fe000
#define PAGE_MASK_16MB				0x01ffe000

#define ENTRYHI_VPN_ODD_NOMASK		0xfffff000		/* 奇偶位不屏蔽 */
#define ENTRYHI_VPN_ODD_MASK		0xffffe000		/* 奇偶位屏蔽*/

#define ENTRYLO_FLAGS_G				0x00000001	/* 全局，忽略ASID */
#define ENTRYLO_FLAGS_V				0x00000002	/* 映射是否有效*/
#define ENTRYLO_FLAGS_D				0x00000004	/* 脏位，同时标记页是否可写 */
#define ENTRYLO_FLAGS_UNCACHED		0x00000010	/* 不带缓存 */
#define ENTRYLO_FLAGS_NONCOHERENT	0x00000018	/* 非一致性缓存 */
#define ENTRYLO_FLAGS_ACCELERATED	0x00000038	/* 非缓存加速 */

#define ENTRYLO_ROPAGE				(ENTRYLO_FLAGS_V | ENTRYLO_FLAGS_NONCOHERENT)
#define ENTRYLO_RWPAGE				(ENTRYLO_FLAGS_V | ENTRYLO_FLAGS_D | ENTRYLO_FLAGS_NONCOHERENT)
#define ENTRYLO_IOPAGE				(ENTRYLO_FLAGS_G | ENTRYLO_FLAGS_V | ENTRYLO_FLAGS_D | ENTRYLO_FLAGS_UNCACHED)

#define ENTRYLO_VPN2_OFFSET			6
//bad codes
#define VADDR_TO_VPN2(vaddr)		(((vaddr) >> 12) & 0x03ffffff)


#endif //__TLB_H__

