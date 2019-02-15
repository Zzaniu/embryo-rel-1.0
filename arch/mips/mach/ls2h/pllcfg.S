/**
 * Copyright(c) 2015-6-5 Shangwen Wu	
 *
 * 系统时钟设置，该时钟设置基于LS2H 
 * 
 */

/*	 
 * 时钟设置步骤为，
 * 选择参考时钟，
 * 关断模式，
 * 设置分频倍频，延时，
 * 离开关断模式，
 * 等待锁定，
 * 切换时钟  
*/

/*
 * 描述：是否硬件设置SYS PLL
 * 返回：v0硬件配置返回非0，反之返回0
 * 使用寄存器v0，a0
 */
LEAF(is_sys_pll_hard_mode)
	la		a0, LS2H_REGS_BASE
	lw		a0, LS2H_CHIP_SAMPLE0(a0)
	and		v0, a0, BOOTCFG_SYSPLL_HARD	
	
	jr		ra
	nop
END(is_sys_pll_hard_mode)

/*
 * 描述：系统时钟配置 void sys_pll_cfg(void)
 * 时钟频率固定
 * 使用寄存器：a0-a2，ra
 */
LEAF(sys_pll_cfg)
	li		a0, LS2H_REGS_BASE		
	li		a1, 0x8f0					/* ldf=56, odf=0 */	
	sw		a1, LS2H_CLOCK_CTRL2(a0)
	li		a1, 0x8f1
	sw		a1, LS2H_CLOCK_CTRL2(a0)	/* 设置更新位 */
	
	li		a2, 0x03
1:
	bnez	a2, 1b
	addu	a2, a2, -1
	
	li 		a1, 0x0f1					/* 离开关断模式 */	
	sw		a1, LS2H_CLOCK_CTRL2(a0)

	li		a2, 0x040
1:
	bnez	a2, 1b						/* 等待锁定 */
	addu	a2, a2, -1

	li		a1, 0x10f1					/* 选择当前时钟 */
	sw		a1, LS2H_CLOCK_CTRL2(a0)
	
	li		a1, 0x10f0					/* 清空更新位 */
	sw		a1, LS2H_CLOCK_CTRL2(a0)
	
	jr		ra
	nop
END(sys_pll_cfg)

/*
 * 描述：是否硬件设置CPU PLL
 * 返回：v0硬件配置返回非0，反之返回0
 * 使用寄存器：v0，a0
 */
LEAF(is_cpu_pll_hard_mode)
	la		a0, LS2H_REGS_BASE
	lw		a0, LS2H_CHIP_SAMPLE0(a0)
	and		v0, a0, BOOTCFG_CPUPLL_HARD	
	
	jr		ra
	nop
END(is_cpu_pll_hard_mode)

#if CPUFREQ == 400
#define CPUPLL_LDF		16
#define CPUPLL_ODF		2
#elif CPUFREQ == 600
#define CPUPLL_LDF		12
#define CPUPLL_ODF		1
#elif CPUFREQ == 800
#define CPUPLL_LDF		16
#define CPUPLL_ODF		1
#elif CPUFREQ == 900
#define CPUPLL_LDF		18
#define CPUPLL_ODF		1
#elif CPUFREQ == 1000
#define CPUPLL_LDF		20
#define CPUPLL_ODF		1
#else
#define CPUPLL_LDF		(CPUFREQ/50)
#define CPUPLL_ODF		1
#endif 

/*
 * 描述：CPU频率配置 void cpu_pll_cfg(void)
 * 时钟频率在autoconf.h中定义
 * 使用寄存器：a0-a3，ra
 */
LEAF(cpu_pll_cfg)
	li		a0, LS2H_REGS_BASE		
	li		a2, (CPUPLL_LDF << CPUPLL_LDF_OFFSET) | (CPUPLL_ODF << CPUPLL_ODF_OFFSET)

	li 		a1, CPUPLL_PD
	or		a1, a1, a2
	sw		a1, LS2H_CLOCK_CTRL0(a0)
	or		a1, a1, CPUPLL_SET
	sw		a1, LS2H_CLOCK_CTRL0(a0)	/* 设置更新位 */
	
	li		a3, 0x03
1:
	bnez	a3, 1b
	addu	a3, a3, -1
	
	or 		a1, a2, 0x1					/* 离开关断模式 */	
	sw		a1, LS2H_CLOCK_CTRL0(a0)

	li		a3, 0x040
1:
	bnez	a3, 1b						/* 等待锁定 */
	addu	a3, a3, -1

	or		a1, a1, CPUPLL_SEL			/* 选择当前时钟 */
	sw		a1, LS2H_CLOCK_CTRL0(a0)
	or		a1, a2, CPUPLL_SEL			/* 清空更新位 */
	sw		a1, LS2H_CLOCK_CTRL0(a0)

	jr		ra
	nop
END(cpu_pll_cfg)

/*
 * 描述：是否硬件设置DDR PLL
 * 返回：v0硬件配置返回非0，反之返回0
 * 使用寄存器：v0，a0
 */
LEAF(is_ddr_pll_hard_mode)
	la		a0, LS2H_REGS_BASE
	lw		a0, LS2H_CHIP_SAMPLE0(a0)
	and		v0, a0, BOOTCFG_DDRPLL_HARD	
	
	jr		ra
	nop
END(is_ddr_pll_hard_mode)

#if DDRFREQ == 400
#define DDRPLL_LDF		8
#define DDRPLL_ODF		1
#define DDRPLL_IDF		2
#elif DDRFREQ == 350
#define DDRPLL_LDF		14	
#define DDRPLL_ODF		2
#define DDRPLL_IDF		2
#elif DDRFREQ == 325
#define DDRPLL_LDF		13
#define DDRPLL_ODF		2
#define DDRPLL_IDF		2
#elif DDRFREQ == 300
#define DDRPLL_LDF		12
#define DDRPLL_ODF		2
#define DDRPLL_IDF		2
#else
#define DDRPLL_LDF		(DDRFREQ/25)
#define DDRPLL_ODF		2
#define DDRPLL_IDF		2
#endif 

/*
 * 描述：DDR频率配置 void ddr_pll_cfg(void)
 * 时钟频率在autoconf.h中定义
 * 使用寄存器：a0-a3，ra
 */
LEAF(ddr_pll_cfg)
	li		a0, LS2H_REGS_BASE		
	li		a2, (DDRPLL_LDF << DDRPLL_LDF_OFFSET) | (DDRPLL_ODF << DDRPLL_ODF_OFFSET) | (DDRPLL_IDF << DDRPLL_IDF_OFFSET)

	li 		a1, DDRPLL_PD
	or		a1, a1, a2
	sw		a1, LS2H_CLOCK_CTRL0(a0)
	or		a1, a1, DDRPLL_SET
	sw		a1, LS2H_CLOCK_CTRL0(a0)	/* 设置更新位 */
	
	li		a3, 0x100
1:
	bnez	a3, 1b
	addu	a3, a3, -1
	
	or 		a1, a2, DDRPLL_SET			/* 离开关断模式 */	
	sw		a1, LS2H_CLOCK_CTRL0(a0)

	li		a3, 0x100
1:
	bnez	a3, 1b						/* 等待锁定 */
	addu	a3, a3, -1

	or		a1, a1, DDRPLL_SEL			/* 选择当前时钟 */
	sw		a1, LS2H_CLOCK_CTRL0(a0)
	or		a1, a2, DDRPLL_SEL			/* 清空更新位 */
	sw		a1, LS2H_CLOCK_CTRL0(a0)

	jr		ra
	nop
END(ddr_pll_cfg)

