
/**
 * Copyright(c) 2015-6-15 Shangwen Wu	
 *
 * LS2H板级相关配置
 * 
 */
#ifndef __MACH_LS2H_H__
#define __MACH_LS2H_H__

#define BOOT_EXCEPTION_VECTOR	0x00400000		/* 启动异常入口点 */
#define CHACHED_MEMORY_ADDR		0x80000000		/* cache的内存区域 */
#define UNCHACHED_MEMORY_ADDR	0xa0000000		/* 未cache的内存区域 */
#define LS2H_MEM_KSEG0			0x80000000  	/* 龙芯2H KSEG0起始地址 */
#define LS2H_MEM_KSEG1			0xa0000000  	/* 龙芯2H KSEG1起始地址 */
#define LS2H_MEM_KSEG2			0xc0000000  	/* 龙芯2H KSEG2起始地址 */
#define LS2H_MEM_KSEG2_SZ		0x40000000		/* 龙芯2H KSEG2区大小 */

/* 龙芯2H-cache大小（16K 4路 cache） */
#define LS2H_CACHE_GODSON2		0x6300
#define LS2H_CACHE_CONFIG		
#define LS2H_CACHE_SIZE_I		0x4000			/* I-cache大小16K */
#define LS2H_CACHE_SIZE_D		0x4000			/* D-cache大小16K */
#define LS2H_CACHE_4WAY							/* 4路相联 */
#define LS2H_CACHE_ECC			0x22

#define LS2H_CACHE_SIZE_L2		0x20000	
#define LS2H_CACHE_4WAY_L2

/* 龙芯2H DDR硬件配置信息，定义见mm/ddr_config_define.h文件 */
/*  
 * NODE ID: 				0(2'b00)
 * Controller: 				MC0(2'b01)
 * Memory Size:				4*512MB(7'b0000100)
 * DDR Width:				x16(1'b1)
 * CS Map:					CS0(4'b0001)
 * Col Size:				2(2'b10)
 * Miror:					standard(1'b0)
 * Bank:					8(1'b1)
 * Max Pins - Addr Pins:	0(2'b00)
 * DIMM Width:				64(1'b0)
 * DIMM Type:				Unbufferd DIMM(1'b0)
 * ECC:						No Ecc(1'b0)
 * DDR Type:				DDR3(2'b11)
 */
#define LS2H_DDR_CONFIG_INFO    0x00000000c0a18404      /* s1 DDR信息配置 */


/* 龙芯2H TLB硬件信息 */
#define LS2H_TLB_ENTRY_SZ		64						/* 64个TLB奇偶项 */		


#endif /* __MACH_LS2H_H__ */
