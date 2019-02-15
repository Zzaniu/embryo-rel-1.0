
/*
 * Copyright(c) 2015-6-16 Shangwen Wu	
 *
 * LS2H DDR控制器配置s1位域定义 
 *
 * s1：保存DDR所有的硬件信息
 * [ 1: 0]: Node ID
 * [ 3: 2]: Controller Select, 2'b00: MC0 and MC1, 2'b01: MC0 only, 2'b10: MC1 only
 * [ 4: 7]: Reserved
 * MC0硬件配置信息
 * [14: 8]: 内存大小，X*512Mb 
 * [15]: DDR数据位宽，1'b1: x16，1'b0: x8    
 * [19:16]: CS MAP???DIMM片选位图
 * [21:20]: 列大小
 * [22]: MIROR???
 * [23]: BANK数：1'b0: 4, 1'b1: 8
 * [26:24]: 实际内存地址引脚数与最大地址引脚数的差值
 * [27]: DIMM位宽 1'b0: 64bits, 1'b1: 32bits
 * [28]: DIMM类型，1'b0: Unbufferd DIMM, 1'b1: Register DIMM
 * [29]: ECC
 * [31:30]: DDR类型，2'b00: NO DIMM，2'b10: DDR2，2'b11: DDR3 
 * MC1配置信息
 * [46:40]: 内存大小，X*512Mb
 * [63:47]: 同MC0的[31:15]
 *
 */

#ifndef __DDR_CONFIG_DEFINE__
#define __DDR_CONFIG_DEFINE__

/* define for s1 */
#define CHIP_NODE_ID_MASK			0x03			
#define CHIP_NODE_ID_OFFSET			0
#define MC_SELECT_MASK				0x0c
#define MC_SELECT_OFFSET			2
#define MEMORY_SIZE_MASK			0x7f00
#define MEMORY_SIZE_OFFSET			8
#define SDRAM_WIDTH_MASK			0x8000	
#define SDRAM_WIDTH_16BITS			0x8000			//x16	
#define SDRAM_WIDTH_OFFSET			15	
#define MC_CS_MAP_MASK				0x0f0000			
#define MC_CS_MAP_OFFSET			16
#define SDRAM_COL_SIZE_MASK			0x300000
#define SDRAM_COL_SIZE_OFFSET		20
#define ADDR_MIRROR_MASK			0x400000
#define ADDR_MIRROR					0x400000
#define ADDR_MIRROR_OFFSET			22
#define SDRAM_EIGHT_BANK_MASK		0x800000
#define SDRAM_EIGHT_BANK			0x800000
#define SDRAM_EIGHT_BANK_OFFSET		23
#define SDRAM_ADDR_PINS_MASK		0x07000000
#define SDRAM_ADDR_PINS_OFFSET		24
#define DIMM_WIDTH_MASK				0x08000000
#define DIMM_WIDTH_64BITS			0x08000000
#define DIMM_WIDTH_OFFSET			27
#define DIMM_TYPE_MASK				0x10000000
#define DIMM_TYPE_RDIMM				0x10000000
#define DIMM_TYPE_OFFSET			28
#define DIMM_ECC_MASK				0x20000000
#define DIMM_ECC					0x20000000
#define DIMM_ECC_OFFSET				29
#define SDRAM_TYPE_MASK				0xc0000000
#define SDRAM_TYPE_OFFSET			30

#define GET_NODE_ID_TO_a1					\
		dli			a1, CHIP_NODE_ID_MASK;	\
		and			a1, s1, a1;				

#define GET_MC_SEL_TO_a1					\
		dli			a1, MC_SELECT_MASK;		\
		and			a1, s1, a1;				\
		dsrl		a1, MC_SELECT_OFFSET

#define MC0_GET_INFO_TO_a1(name)			\
		dli			a1, ##name##_MASK;		\
		and			a1, s1, a1;				\
		dsrl		a1, ##name##_OFFSET

#define MC1_GET_INFO_TO_a1(name)					\
		andi		a1, s1, ##name##_MASK << 32;	\
		dsrl		a1, ##name##_OFFSET + 32


/* DDR相关寄存器，使用XKPHY空间地址，不使用缓存，使用0xbfd0xxxx应该也行 */
#define DDR_CONFIG_SPACE_REG			0x900000001fd00200		/* 配置空间打开寄存器 */
#define DDR_ENABLE_CONFIG_SPACE			0x00004000				/* 打开配置空间 */
#define DDR_DISABLE_CONFIG_SPACE		0xffffbfff				/* 关闭配置空间 */
#define DDR_CONFIG_REG_BASE				0x900000000ff00000		/* 配置寄存器基址 */

/* 获取当前节点对应的配置地址 */
#define GET_CUR_NODE_CONFIG_REG_TO_t7				\
		dli			t7, CHIP_NODE_ID_MASK;			\
		and			t7, s1, t7;						\
		dsll		t7, 44;							\
		or			t7, DDR_CONFIG_REG_BASE

#endif /* __DDR_CONFIG_DEFINE__ */

