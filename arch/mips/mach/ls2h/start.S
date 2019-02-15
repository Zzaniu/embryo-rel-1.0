
/**
 * Copyright(c) 2015-6-5 Shangwen Wu	
 *
 * embryo-bios基于MIPS架构的入口文件 
 *
 * "embryo" ———— 萌芽、新生之意
 * 
 */

#include <autoconf.h>
#include <common.h>
#include <asm/regdef.h>
#include <asm/cpu.h>
#include <asm/asm.h>
#include <asm/tlb.h>
#include <mach/ls2h.h>
#include <mach/ls2h_regs.h>
#include <mach/ns16550.h>

/* 输出字符串 */
#define PRINTSTR(str)						\
	.rdata; 101: .asciz str; .text; la a0, 101b; bal serial_puts; nop

#define DEBUG_PMON_ON_BOOT					//调试开关
#ifdef DEBUG_PMON_ON_BOOT
#define TTY_DEBUG(str)						\
	.rdata; 101: .asciz str; .text; la a0, 101b; bal serial_puts; nop
#else
#define TTY_DEBUG(str)	;	
#endif

	.set	noreorder
	.global _start	
	.set    mips64   	                   /* 以下代码将使用到64位地址空间和数据，因此需使能MIPS64指令版本 */

/*
 * 某些寄存器指定用途	
 *
 * s0：存放uncached（kseg1）地址空间与程序地址的相对偏移，用于访问cache初始化之前的内存地址数据
 * s1：保存DDR硬件配置信息
 *
 */

/* 以下向量处理程序代码不要超过指定长度，并且按地址要求存放，否则将导致某些中断向量无法运行 */
_start:
start:

	.global	stack
stack = start - 0x4000						/* 设置C环境栈地址，栈为start-16K以下区域 */

/********************* 硬复位异常向量：0xbfc00000 *********************/	
	mtc0		zero, CP0_SR				/*  清零SR寄存器主要是为了屏蔽所有中断 */	
	mtc0		zero, CP0_CAUSE				/*  清零中断原因寄存器 */	
	li			t0, BOOT_EXCEPTION_VECTOR
	mtc0		t0, CP0_SR					/*  设置异常向量为boot模式 */	
	bal			initregs					/* 因为采用立即寻址，所以跳转到程序地址也没关系，初始化寄存器 */	
	nop
	bal			uncached					/* 计算uncached（kseg1）的地址空间 */
	nop
	bal 		locate
	nop

uncached:									/* uncached必须紧跟在locate函数调用之后 */
	or			ra, UNCHACHED_MEMORY_ADDR
	jr			ra							/* 确保程序执行行在kseg1段，有点多余，ra实际上已经指向kseg1，而非程序地址 */
	nop	
	
/******************** 高位存储器映射和软复位异常向量：0xbfc00100 ********************/	
	.align		8							/* 以2的8次方对齐 */
map_reset_exc_vector:
	PRINTSTR("Bios panic! Unexpected memory mapping soft reset exception!\r\n")	
	bal			exc_common
	nop

/******************** TLB重填异常向量：0xbfc00200 ********************/	
	.align		8				
tlb_exc_vector:
	PRINTSTR("Bios panic! Unexpected TLB refill exception!\r\n")	
	bal			exc_common
	nop

/******************** XTLB重填异常向量：0xbfc00280 ********************/	
	.align		7				
xtlb_exc_vector:
	PRINTSTR("Bios panic! Unexpected XTLB refill exception!\r\n")	
	bal			exc_common
	nop

/******************** cache 错误异常向量：0xbfc00300 ********************/	
	.align		8				
cache_exc_vector:
	PRINTSTR("Bios panic! Cache error exception!\r\n")	
	bal			exc_common
	nop

/******************** 一般异常向量：0xbfc00380 ********************/	
	.align		7
general_exc_vector:
	PRINTSTR("Bios panic! Unexpected general exception!\r\n")	
	bal			exc_common
	nop

/******************** 中断异常向量：0xbfc00400 ********************/	
	.align		8				
int_exc_vector:
	PRINTSTR("Bios panic! Unexpected interrupt!\r\n")	
	bal			exc_common
	nop

/******************** 调试模式异常向量：0xbfc00480 ********************/	
	.align		7				
debug_exc_vector:
	PRINTSTR("Enter debug mode...\r\n")	
	bal			exc_common
	nop

exc_common:
	PRINTSTR("Dump Registers Info:\r\n")	
	PRINTSTR("Register STATUS: 0x")
	mfc0		a0, CP0_SR	
	bal			serial_puthex			
	nop			
	PRINTSTR("\r\n")	
	PRINTSTR("Register CAUSE: 0x")
	mfc0		a0, CP0_CAUSE	
	bal			serial_puthex			
	nop			
	PRINTSTR("\r\n")	
	PRINTSTR("Register ERRORPC: 0x")
	mfc0		a0, CP0_ERROREPC	
	bal			serial_puthex			
	nop			
	PRINTSTR("\r\n")	
	PRINTSTR("Register EPC: 0x")
	mfc0		a0, CP0_EPC
	bal			serial_puthex			
	nop			
	PRINTSTR("\r\n")	
	PRINTSTR("Embryo abort with error!!!\r\n")	
1:
	b			1b
	nop

	.align 		8							/* 将后续代码进行对齐 */
	nop
	.align 		8
	nop

/* 计算程序地址与执行的实际地址的偏移，将结果保存到s0中 */
locate:
	la 			s0, uncached				/* uncache保存的是程序地址 */
	subu		s0, ra, s0					/* s0 == 0x30300000 */

	/* 判断是否由硬件设置SYS和CPU时钟，否则采用软件配置 */
	bal			is_sys_pll_hard_mode		
	nop
	bnez		v0, 1f
	nop
	bal			sys_pll_cfg
	nop				
1:
	bal			is_cpu_pll_hard_mode		
	nop
	bnez		v0, 2f
	nop
	bal			cpu_pll_cfg
	nop				
2:
	/* 初始化系统访问地址映射窗口 */
#include "xbar_win_init.S"

	bal 		initserial					/* 初始化串口 */
	nop			
	PRINTSTR("\r\nBoot serial init done.\r\n")
	/* 判断是否由硬件设置DDR时钟，否则采用软件配置 */
	bal			is_ddr_pll_hard_mode		
	nop
	bnez		v0, 1f
	nop
	bal			ddr_pll_cfg
	nop				
	PRINTSTR("software set ddr pll done\r\n")
	b			2f
	nop
1:
	PRINTSTR("set ddr pll by hardware\r\n")

2:
	bal			i2c_init
	nop		
	PRINTSTR("i2c controller init done\r\n")

/* 缓存初始化 */
	mfc0		t0, CP0_PRID
	andi		t0, 0xff00
	bne			t0, LS2H_CACHE_GODSON2, l1_cache_done
	nop
	TTY_DEBUG("target board use godson2 caches!\r\n")
	bal			godson2_cache_init
	nop
	PRINTSTR("godson2-cache(L1-cache) init done\r\n")

l1_cache_done:
	bal			l2_cache_init
	nop
	PRINTSTR("L2-cache init done\r\n")

	mfc0		t0, CP0_CONFIG
	and			t0, 0xfffffff8
	ori			t0, CONFIG_KSEG0_CACHABLE	/* 使能SEG0缓存 */
	mtc0		t0, CP0_CONFIG

	bal			cached
	nop
	subu		s0, 0x20000000				/* 重新调整程序地址与执行地址的偏移 */	
	TTY_DEBUG("switch cached address space...\r\n")

/* 内存初始化 */
#include "mm/ddr_config_define.h"
#include "mm/ddr_param_define.h"

	dli         s1, LS2H_DDR_CONFIG_INFO

#include "mm/ls2h_ddr_config.S"
 
#if 0							//bug I2C从设备无响应
	/* 保存DIMM SPD信息 */
	bal			store_spd_info
	nop 
#endif

	/* 将BIOS代码段以及数据段从当前ROM空间中拷贝到内存空间（地址就是程序的执行地址） */	
	dla			t0, start		
	dla			t1, edata
	dla			t2, end
	dla			t3, 0xffffffffbfc00000				/* 程序的加载地址，当前程序位于ROM空间 */
	
	/* 注意：下面的操作将采用uncache区域，若使用当前地址区域（已缓存），则必须使用synci保证数据正确 */
	or			t0, UNCHACHED_MEMORY_ADDR
	or			t1,	UNCHACHED_MEMORY_ADDR	
	or			t2,	UNCHACHED_MEMORY_ADDR	

#ifdef DEBUG_PMON_ON_BOOT
	TTY_DEBUG("embryo ld info:")
	TTY_DEBUG("\r\n _start = 0x")
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	TTY_DEBUG("\r\n _edata = 0x")
	dsrl		a0, t1, 32
	bal			serial_puthex
	nop
	move		a0, t1
	bal			serial_puthex
	nop
	TTY_DEBUG("\r\n _end = 0x")
	dsrl		a0, t2, 32
	bal			serial_puthex
	nop
	move		a0, t2
	bal			serial_puthex
	nop
	TTY_DEBUG("\r\n")
#endif

	PRINTSTR("copy empbryo to memory... ")

#ifdef DEBUG_PMON_ON_BOOT
	TTY_DEBUG("\r\ncopy text section from address 0x")
	dsrl		a0, t3, 32
	bal			serial_puthex
	nop
	move		a0, t3
	bal			serial_puthex
	nop
	TTY_DEBUG(" to address 0x")
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	TTY_DEBUG("...\r\n")
#endif
	
1:
	lw			a0, 0x00(t3)	
	nop													//保险起见
	sw			a0, 0x00(t0)
	daddu		t3, 0x04
	bltu		t0, t1, 1b
	daddiu		t0, t0, 0x04

	TTY_DEBUG("clear bss section...\r\n")

1:
	sw			zero, 0x00(t1)	
	bltu		t1, t2, 1b
	daddiu		t1, t1, 0x04

	PRINTSTR("done!\r\n")

	/* 进入C环境！！！ */
	move		a0, s2						//s2保存着调用初始化内存之后计算出来的内存大小 
	dla			a1, initmips		
	jalr		a1
	nop

/*************************** 函数定义 ***************************/

/************************** TLB相关 **************************/
#ifndef MTC0
#if __mips64
	#define MTC0	dmtc0
#else
	#define MTC0	mtc0
#endif
#endif

/*
 * 描述：清除TLB表项
 * 注意：embryo下，TLB初始化采用32位兼容模式
 *
 */
LEAF(tlb_clear)
	dli			a0, 0					//index由0到63
	dli			a1, LS2H_TLB_ENTRY_SZ	

//	li			a2, PAGE_MASK_4KB		//note: PMON的页设置为4KB
	li			a2, PAGE_MASK_16MB
	MTC0		a2, CP0_PAGEMASK
	MTC0		zero, CP0_WIRED			//非禁锢
	
1:
	MTC0		zero, CP0_ENTRYHI
	MTC0		zero, CP0_ENTRYLO0
	MTC0		zero, CP0_ENTRYLO1
	mtc0		a0, CP0_INDEX

//	ehb									//note: PMON使用的nop，MIPS64不支持ehb，所以这里采用延时

	daddiu		a0, a0, 1
	nop
	nop
	nop
	tlbwi			
	
	blt			a0, a1, 1b
	nop
	
	jr			ra
	nop
END(tlb_clear)

/*
 * 描述：TLB初始化，该函数将对KSEG2区域进行页大小为16MB的初始化设置
 * 注意：embryo不涉及64位机映射模式，该初始化将导致KSEG2将全部静态写入到TLB中，
 *	     并且，TLB还有一半空间未被使用，TLB总共可存放16MB*128=2G空间的虚拟映射
 *       。该函数将0xc0000000映射到位置0xff000000。可能有BUG！
 */
LEAF(tlb_init)
	li			a2, 0					//index由0开始

	li			a3, PAGE_MASK_16MB
	MTC0		a3, CP0_PAGEMASK
	MTC0		zero, CP0_WIRED			//非禁锢

1:
	and			a3, a0, ENTRYHI_VPN_ODD_NOMASK 
	MTC0		a3, CP0_ENTRYHI
	//subu		a3, a0, 0xa0000000		//由0xc0000000~0xffffffff映射到0x20000000~0x5fffffff
	srl			a3, a0, 12				//bad code
	sll			a3, 6
	and			a3, 0x3fffffc0			//不明白的映射
	or			a3, ENTRYLO_IOPAGE		//映射为可读写的IO页（不带缓存）
	MTC0		a3, CP0_ENTRYLO0
	addu		a3, (SZ_16M >> 6)		//映射奇页
	MTC0		a3, CP0_ENTRYLO1
	
	mtc0		a2, CP0_INDEX			//官方的PMON版本在最后一次的CP0操作后，插入四条指令后才进行tlbwi以防止遇险

	addu		a2, 1
	subu		a1, SZ_16M * 2			//奇偶项
	addu		a0, SZ_16M * 2
	nop
	
	tlbwi		

	bgtz		a1, 1b	
	nop

	jr			ra
	nop
END(tlb_init)

/************************** DDR相关 **************************/
/*
 * 描述：将DIMM SPD信息写入到内存指定区域，当前I2C从地址的0x02寄存器小于
 *		 0x80时写入0x0~0x100寄存器的值到内存，否则，写入0
 *
 */
#define DIMM_SPD_INFO_I2C_SLAVE_BEGIN			0x54
#define DIMM_SPD_INFO_I2C_SLAVE_END				0x56
#define DIMM_SPD_INFO_STORE_ADDRESS				0xffffffff8fffa000

LEAF(store_spd_info)
	move		t9, ra
	TTY_DEBUG("store spd info...")
	dli			t3, DIMM_SPD_INFO_I2C_SLAVE_BEGIN
	dli			t4, DIMM_SPD_INFO_I2C_SLAVE_END
	dli			t5, DIMM_SPD_INFO_STORE_ADDRESS
	
1:	
	move		a0, t3
	dli			a1, 0x02	
	bal			i2c_read_byte
	nop
	dli			a2, 0x80
	bltu		v0, a2, 3f
	nop
	//清零0x0~0x100
2:
	move		t0, t5
	daddu		t1, t5, 0x100
	sd			zero, 0x00(t0)
	daddu		t0, 0x01
	bltu		t0, t1, 2b
	nop
	b			5f
	nop
3:
	//写入寄存器0x0~0x100到内存
	move		t2, t5
	move		t0, zero
	dli			t1, 0x100
4:
	move		a0, t3
	move		a1, t0
	bal			i2c_read_byte	
	nop	
	sd			v0, 0x00(t2)
	daddu		t2, 0x01
	daddu		t0, 0x01
	bltu		t0, t1, 4b
	nop
	//下一个从地址	
5:	
	daddu		t5, 0x100
	daddu		t3, 0x02
	bltu		t3, t4, 1b
	nop

	TTY_DEBUG(" done!\r\n")
	jr			t9
	nop
END(store_spd_info)

/************************** cache相关 **************************/
/*
 * 描述：将程序执行地址切换到kseg0段，使用cache
 *
 */
cached:
	li			a0, 0xdfffffff
	and			ra, a0
	jr			ra
	nop

/*
 * 描述：初始化godson2-cache，cache在config寄存器中的描述跟标准MIPS架构不同，
 * 并且2H的CPU计算方式很奇怪，故采用硬配置的方式进行计算cache大小  
 * v0：KSEG0基地值，v1：cache填充区域结束地址，a0：ECC，a1：D-cache大小，a2：I-cache大小 
 */
LEAF(godson2_cache_init)
/* 将KSEG0空间的低地址的一段区域填充到CACHE的TAG域 */
#ifdef LS2H_CACHE_CONFIG
/* D-cache初始化 */	
	li			a0, LS2H_CACHE_ECC
	mtc0		a0, CP0_ECC					/* ？？？ */	
	mtc0		zero, CP0_TAGHI
	mtc0		zero, CP0_TAGLO
	la			v0, LS2H_MEM_KSEG0
	addu		a1, zero, LS2H_CACHE_SIZE_D			/* 16k-4way */
	addu		v1, v0, a1
1:
	cache		Index_Store_Tag_D, 0x0(v0)
	cache		Index_Store_Tag_D, 0x1(v0)
#ifdef LS2H_CACHE_4WAY
	cache		Index_Store_Tag_D, 0x2(v0)
	cache		Index_Store_Tag_D, 0x3(v0)
#endif
	blt			v0, v1, 1b
	addu		v0, 0x20					/* 缓存行长度32B */
	
/* I-cache初始化 */	
	mtc0		zero, CP0_ECC	
	la			v0, LS2H_MEM_KSEG0
	addu		a1, zero, LS2H_CACHE_SIZE_I		/* 16k-4way */
	addu		v1, v0, a1
1:
	cache		Index_Store_Tag_I, 0x0(v0)
	cache		Index_Store_Tag_I, 0x1(v0)
#ifdef LS2H_CACHE_4WAY
	cache		Index_Store_Tag_I, 0x2(v0)
	cache		Index_Store_Tag_I, 0x3(v0)
#endif
	blt			v0, v1, 1b
	addu		v0, 0x20					/* 缓存行长度32B */
#else				
	/* 读取CP0_CONFIG配置cache方式预留 */					
#endif	/* LS2H_CACHE_CONFIG */
	
	jr			ra
	nop
END(godson2_cache_init)

/*
 * 描述：初始化L2-cache，L2-cache在config寄存器中的描述跟标准MIPS架构不同，
 * 并且2H的cache大小计算方式很奇怪，故采用硬配置的方式进行计算cache大小  
 * v0：KSEG0基地值，v1：cache填充区域结束地址，a0：ECC，a1：L2-cache大小
 */
LEAF(l2_cache_init)
#ifdef LS2H_CACHE_CONFIG
	mtc0		zero, CP0_TAGLO
	mtc0		zero, CP0_TAGHI
	li			a0, LS2H_CACHE_ECC
	mtc0		a0, CP0_ECC					/* ？？？ */	
	la			v0, LS2H_MEM_KSEG0
	li			a1, LS2H_CACHE_SIZE_L2		/* 512K/4way */
	addu		v1, v0, a1
1:
	cache		Index_Store_Tag_S, 0x0(v0)
	cache		Index_Store_Tag_S, 0x1(v0)
#ifdef LS2H_CACHE_4WAY_L2
	cache		Index_Store_Tag_S, 0x2(v0)
	cache		Index_Store_Tag_S, 0x3(v0)
#endif
	blt			v0, v1, 1b
	addu		v0, 0x20					/* 缓存行长度8B？ */
#else				
	/* 读取CP0_CONFIG配置cache方式预留 */					
#endif	/* LS2H_CACHE_CONFIG */

	jr			ra
	nop
END(l2_cache_init)

/*************************** 串口相关***************************/
#define UART_REGS_BASE		_UART_REGS_BASE(BIOS_CONSOLE_UART)
/*
 * Copyright(c) NS16550 
 * 描述：该函数用于初始化NS16550兼容串口
 * 寄存器使用：v0，v1， a0 
 *
 */
LEAF(initserial)							/* void initserial(void) */
	.set		noat
	move		AT, ra

	/* 使能多串复用 */
#if UART_SPLIT_ENABLE 
	la			v0, LS2H_CHIP_CONFIG0_REG
	lw			a0, 0x0(v0)
	ori			a0, 0x03
	sw			a0, 0x0(v0)
	la			v0, LS2H_GPIOCFG_REG 
	lw			a0, 0x0(v0)
	ori			a0, 0xfc
	sw			a0, 0x0(v0)
	sync
#endif

	la 			v0, UART_REGS_BASE 
	li			a0,	(125000000/16)/CONSOLE_UART_BAUDRATE
	nop
	bal 		1f
	nop
	jr			AT
	nop
1:
	li			v1, NS16550_FCR_FIFO|NS16550_FCR_TL_4BYTE|NS16550_FCR_TXRST|NS16550_FCR_RXRST
	sb			v1, NS16550_FCR(v0)	
	li			v1, NS16550_LCR_DLAB			/* 注意：这里必须先设置分频，在设置其他参数 */
	sb			v1, NS16550_LCR(v0)
	sb			a0, NS16550_PRELO(v0)	
	srl			a0, 8
	sb			a0, NS16550_PREHI(v0)	
#if !UART_SPLIT_ENABLE
	li			v1, NS16550_MCR_RTSC|NS16550_MCR_DTRC
#endif
	sb			v1, NS16550_MCR(v0)	
	li			v1, NS16550_LCR_BCC_8BIT
	sb			v1, NS16550_LCR(v0)	
	li			v1, 0x0
	sb			v1, NS16550_IER(v0)
	
	jr 			ra
	nop
	.set		at
END(initserial)

/*
 * 描述：串口输出字符串
 * 参数：a0-要发送的字符串首地址（程序地址）
 * 寄存器使用：a0，a1，v0，v1
 */
LEAF(serial_puts)							/* void serial_puts(char *) */	
	.set		noat
	move		AT, ra
	addu		a1, a0, s0					/* 注意字符串地址需加上之前计算的偏移值 */

	lb			a0, 0(a1)			
1:
	beqz		a0, 2f
	nop
	bal			serial_putc			
	addu		a1, 1
	b			1b
	lb			a0, 0(a1)			

2:
	jr 			AT
	nop
	.set		at
END(serial_puts)

/*
 * 描述：串口输出变量的十六进制值（低32位）
 * 参数：a0需要输出的变量
 * 寄存器使用：a0，a1，a2，a3，v0，v1
 */
LEAF(serial_puthex)							/* void serial_puts(char *) */	
	move		a3, ra

	move		a1, a0
	li			a2, 28
	srl			a0, a1, a2
1:
	andi		a0, 0x0f
	blt			a0, 0x0a, 2f
	addu		a0, 48						/* 转换为对应ASCII码 */
	addu		a0, 39
2:
	bal			serial_putc			
	addu		a2, -4
	bne			a2, -4, 1b
	srl			a0, a1, a2
	
	jr 			a3
	nop
END(serial_puthex)

/*
 * 描述：NS16550兼容串口输出一个字符
 * 参数：a0-要发送的字符值
 * 使用寄存器：v0，v1，a0
 */
LEAF(serial_putc)							/* void serial_putc(c) */
	la			v0, UART_REGS_BASE
	andi		a0, 0x00ff
1:
	lbu			v1, NS16550_LSR(v0)			/* wait idle */
	andi		v1, NS16550_LSR_TE
	beqz		v1, 1b
	nop										/* bug：使用缓存后，这里必须加上空操作，否则将出现异常 */
	sb			a0, NS16550_DAT(v0)

	jr			ra
	nop
END(serial_putc)

/************************* CPU寄存器初始化*************************/
initregs:
	.set		noat
	move		AT, zero					/* 初始化通用寄存器 */
	move		v0, zero
	move		v1, zero
	move		a0, zero
	move		a1, zero
	move		a2, zero
	move		a3, zero
	move		t0, zero
	move		t1, zero
	move		t2, zero
	move		t3, zero
	move		t4, zero
	move		t5, zero
	move		t6, zero
	move		t7, zero
	move		s0, zero
	move		s1, zero
	move		s2, zero
	move		s3, zero
	move		s4, zero
	move		s5, zero
	move		s6, zero
	move		s7, zero
	move		t8, zero
	move		t9, zero
	move		k0, zero
	move		k1, zero
	move		sp, zero
	move		gp, zero
	move		fp, zero
	.set		at

	mfc0		t0, CP0_SR 
	or			t0, t0, 0x640000e0|BOOT_EXCEPTION_VECTOR
	mtc0		t0, CP0_SR	

	dla			sp, stack					/* 栈地址设置 */
	dla			gp, _gp						/* 全局区地址设置 */

	jr 			ra
	nop

/*************************** 包含其他依赖汇编文件 ***************************/

/* 时钟配置函数集 */
#include "pllcfg.S"

/* I2C模块操作 */
#include "ls2h_i2c.S"

/* DDR函数集 */
#include "mm/ls2h_ddr_func.S"							/* 包含DDR配置函数 */
#include "mm/ddr_param_modify_func.S"					/* 包含DDR参数修改函数 */

	.align	5
	.text		
	.global	mc0_level_info								/* 在检查是否需要level时，需要用到 */
	.global	ddr_reg_data_mc0_leveled					/* 已校准的配置信息 */
	.global	ddr2_UDIMM_reg_data_mc0						/* 未校准的配置信息 */
	.global	ddr2_RDIMM_reg_data_mc0						/* 未校准的配置信息 */
	.global	ddr3_UDIMM_reg_data_mc0						/* 未校准的配置信息 */
	.global	ddr3_RDIMM_reg_data_mc0						/* 未校准的配置信息 */

#include "mm/ls2h_ddr_param_leveled.S"					/* 包含已校准的DDR配置参数信息 */
#include "mm/ls2h_ddr_param.S"							/* 包含未校准的DDR配置参数信息 */

	.set 	reorder
	.set 	mips3

