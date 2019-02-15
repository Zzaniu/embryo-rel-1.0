
/**
 * Copyright(c) 2015-6-5 Shangwen Wu	
 *
 * MIPS架构的协处理器定义以及操作定义
 * 
 */

#ifndef __ASM_CPU_H__
#define	__ASM_CPU_H__

/* 协处理器CP0寄存器定义 */
#define CP0_INDEX		$0					/* TLB索引寄存器 */
#define CP0_RANDOM		$1					/* TLB索引随机生成寄存器 */
#define CP0_ENTRYLO0	$2					/* TLB偶项低位 */
#define CP0_ENTRYLO1	$3					/* TLB奇项低位 */
#define CP0_CONTEXT		$4					/* TLB页表便利寄存器 */
#define CP0_PAGEMASK	$5					/* TLB PageMask寄存器 */
#define CP0_WIRED		$6					/* TLB禁锢寄存器 */
#define CP0_COUNT		$9					/* 片上计数寄存器 */
#define CP0_ENTRYHI		$10					/* TLB项高位 */
#define CP0_SR			$12					/* CPU控制寄存器 */
#define	CP0_CAUSE		$13					/* 中断原因寄存器 */
#define CP0_EPC			$14					/* 中断返回地址寄存器 */
#define CP0_PRID		$15					/* CPU类型版本号 */
#define CP0_CONFIG		$16					/* CPU硬件配置信息 */
#define CP0_WATCH_LO	$18					/* WATCH低位 */
#define CP0_WATCH_HI	$19					/* WATCH高位 */
#define CP0_XCONTEXT	$20					/* 64位机TLB页表便利寄存器 */
#define CP0_ECC			$26					/* cache ECC寄存器 */
#define CP0_TAGLO		$28					/* cache标签寄存器低位 */
#define CP0_TAGHI		$29					/* cache标签寄存器高位 */
#define CP0_ERROREPC	$30					/* 中断返回地址寄存器 */

/* 状态寄存器相关字段 */
#define SR_CU0_ENA		0x10000000			/* 协处理器0使能（默认使能） */
#define SR_CU1_ENA		0x20000000			/* 协处理器1使能 */
#define SR_FR32			0x04000000			/* 使能32个64位宽浮点寄存器 */
#define SR_EXL			0x00000002			/* CPU进入异常级别 */

/* CONFIG kseg0 cache配置 */
#define CONFIG_KSEG0_CACHABLE		0x03	/* 开启kseg0段cache */
#define CONFIG_KSEG0_UNCACHED		0x02	/* 关闭kseg0段cache */
#define CONFIG_KSEG0_ACCELERATED	0x07	/* 开启kseg0段非cache加速功能 */

/*
 * 龙芯2H CPU兼容cache操作
 */ 
#define	Index_Invalidate_I		0x00		/* I-cache索引作废 */	
#define	Index_Store_Tag_I		0x08		/* I-cache保存TAG */	
#define	Index_Store_Tag_D		0x09		/* D-cache保存TAG */	
#define	Index_Store_Tag_S		0x0b		/* 二级cache保存TAG */	
#define	Index_WriteBack_Inv_D	0x01		/* D-cache索引回写作废 */	
#define	Index_WriteBack_Inv_S	0x01		/* 二级cache索引回写作废 */	
#define Hit_WriteBack_Inv_D		0x15		/* D-cache命中回写作废 */
#define Hit_WriteBack_Inv_S		0x17		/* 二级cache命中回写作废 */
#define Hit_Invalidate_D		0x11		/* D-cache命中作废 */
#define Hit_Invalidate_S		0x13		/* 二级cache命中作废 */
#define	Invalidate_Page_S		0x17		/* 作废片外二级cache，Only RM527[0-1] */
#define	Invalidate_Page_T		0x16		/* 作废三级cache，Only RM7K */

/*
 * MIPS CPU types (cp_imp).CPU类型标识 CP0_PRID
 */
#define MIPS_CPU_MASK	0xff00
#define	MIPS_R2000		0x01	/* MIPS R2000 CPU		ISA I   */
#define	MIPS_R3000		0x02	/* MIPS R3000 CPU		ISA I   */
#define	MIPS_R6000		0x03	/* MIPS R6000 CPU		ISA II	*/
#define	MIPS_R4000		0x04	/* MIPS R4000/4400 CPU		ISA III	*/
#define MIPS_R3LSI		0x05	/* LSI Logic R3000 derivate	ISA I	*/
#define	MIPS_R6000A		0x06	/* MIPS R6000A CPU		ISA II	*/
#define	MIPS_R3IDT		0x07	/* IDT R3000 derivate		ISA I	*/
#define	MIPS_R10000		0x09	/* MIPS R10000/T5 CPU		ISA IV  */
#define	MIPS_R4200		0x0a	/* MIPS R4200 CPU (ICE)		ISA III */
#define MIPS_R4300		0x0b	/* NEC VR4300 CPU		ISA III */
#define MIPS_R4100		0x0c	/* NEC VR41xx CPU MIPS-16	ISA III */
#define	MIPS_R8000		0x10	/* MIPS R8000 Blackbird/TFP	ISA IV  */
#define	MIPS_R4600		0x20	/* QED R4600 Orion		ISA III */
#define	MIPS_R4700		0x21	/* QED R4700 Orion		ISA III */
#define	MIPS_R3TOSH		0x22	/* Toshiba R3000 based CPU	ISA I	*/
#define	MIPS_R5000		0x23	/* MIPS R5000 CPU		ISA IV  */
#define	MIPS_RM7000		0x27	/* QED RM7000 CPU		ISA IV  */
#define	MIPS_RM52X0		0x28	/* QED RM52X0 CPU		ISA IV  */
#define MIPS_E9000		0x34	/* PMC-Sierra E9000 core (RM9k) ISA IV  */
#define	MIPS_VR5400		0x54	/* NEC Vr5400 CPU		ISA IV+ */
#define MIPS_GODSON2	0x63	/* Godson 2 CPU */
#define MIPS_GODSON1	0x42    /* Godson 1 CPU */

#define CACHE_TYPE_WAY_MASK		0xff
#define CACHE_TYPE_DIR			0x0		/* 单路组相联cache */
#define CACHE_TYPE_2WAY			0x1		/* 2路组相联cache */
#define CACHE_TYPE_4WAY			0x2		/* 4路组相联cache */
#define CACHE_TYPE_8WAY			0x3		/* 8路组相联cache */
#define CACHE_TYPE_HAS_L2		0x100	/* 是否有片上二级cache */
#define CACHE_TYPE_HAS_XL2		0x200	/* 是否有片外二级cache */
#define CACHE_TYPE_HAS_L3		0x400	/* 是否有三级cache */

#define CPU_KSEG_CACHED			0x80000000			/* cache的内存区域 */
#define CPU_KSEG_UNCACHED		0xa0000000			/* 未cache的内存区域 */

/* KSEG0/1地址空间与物理地址的相互转换 */
#ifndef _LOCORE
#define PHY_TO_UNCACHED(a)		((unsigned long)(a) | CPU_KSEG_UNCACHED)
#define PHY_TO_CACHED(a)		((unsigned long)(a) | CPU_KSEG_CACHED)
#define UNCACHED_TO_PHY(a)		((unsigned long)(a) & 0x1fffffff)
#define CACHED_TO_PHY(a)		((unsigned long)(a) & 0x1fffffff)
#else
#define PHY_TO_UNCACHED(a)		((a) | CPU_KSEG_UNCACHED)
#define PHY_TO_CACHED(a)		((a) | CPU_KSEG_CACHED)
#define UNCACHED_TO_PHY(a)		((a) & 0x1fffffff)
#define CACHED_TO_PHY(a)		((a) & 0x1fffffff)
#endif

#define CACHED_TO_UNCACHED(a)	(PHY_TO_UNCACHED(CACHED_TO_PHY(a)))
#define UNCACHED_TO_CACHED(a)	(PHY_TO_CACHED(UNCACHED_TO_PHY(a)))
#define PHY_TO_VA				PHY_TO_CACHED
#define VA_TO_PHY				CACHED_TO_PHY

#endif /* __ASM_CPU_H__ */

