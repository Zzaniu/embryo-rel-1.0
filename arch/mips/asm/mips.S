
/**
 * Copyright(c) 2016-2-4 Shangwen Wu	
 *
 * MIPS架构相关的汇编函数库
 * 
 */
#include <asm/regdef.h>
#include <asm/cpu.h>
#include <asm/asm.h>
#include <mach/trapframe.h>

	.set		noreorder

/**
 * void __go(void)
 * 描述：跳转到客户程序地址，该函数永不返回
 *		 实际上该函数是个异常处理函数
 * 使用寄存器：v0, v1, k0, k1
 */
LEAF(__go)	
	/* 下面的很多指令错序编排，为的是减少不必要的加载等待时间 */
	/* 进入异常级别 */
	li 			v0, SR_EXL
	mtc0		v0, CP0_SR	
	
	/**
	 * 下面之所以可以直接使用li/la等32位操作指令，是因为即使在64位情况下，
	 * 这些32位指令仍能在向64位扩展过程中保证正确的兼容性 
	 */
	la			k0, bootcore_regsframe

	LOAD		v0, MULLO * RSIZE(k0)
	LOAD		v1, MULHI * RSIZE(k0)
	mtlo		v0
	mthi		v1

	LOAD		v0, CAUSE * RSIZE(k0)
	LOAD		v1, EPC * RSIZE(k0)
	mtc0		v0, CP0_CAUSE
	/* 注意：对于64位机器应当使用64位指令，因为EPC寄存器位宽与机器相关 */
	MTC0  		v1, CP0_EPC
	
	LOAD		v0, SR * RSIZE(k0)
	ori			v0, SR_EXL
	mtc0		v0, CP0_SR
	
	LOAD		v0, V0 * RSIZE(k0)
	LOAD		v1, V1 * RSIZE(k0)
	LOAD		a0, A0 * RSIZE(k0)
	LOAD		a1, A1 * RSIZE(k0)
	LOAD		a2, A2 * RSIZE(k0)
	LOAD		a3, A3 * RSIZE(k0)
	LOAD		t0, T0 * RSIZE(k0)
	LOAD		t1, T1 * RSIZE(k0)
	LOAD		t2, T2 * RSIZE(k0)
	LOAD		t3, T3 * RSIZE(k0)
	LOAD		t4, T4 * RSIZE(k0)
	LOAD		t5, T5 * RSIZE(k0)
	LOAD		t6, T6 * RSIZE(k0)
	LOAD		t7, T7 * RSIZE(k0)
	LOAD		s0, S0 * RSIZE(k0)
	LOAD		s1, S1 * RSIZE(k0)
	LOAD		s2, S2 * RSIZE(k0)
	LOAD		s3, S3 * RSIZE(k0)
	LOAD		s4, S4 * RSIZE(k0)
	LOAD		s5, S5 * RSIZE(k0)
	LOAD		s6, S6 * RSIZE(k0)
	LOAD		s7, S7 * RSIZE(k0)
	LOAD		t8, T8 * RSIZE(k0)
	LOAD		t9, T9 * RSIZE(k0)
	LOAD		k1, K1 * RSIZE(k0)
	LOAD		gp, GP * RSIZE(k0)
	LOAD		sp, SP * RSIZE(k0)
	LOAD		s8, S8 * RSIZE(k0)
	LOAD		ra, RA * RSIZE(k0)
	.set		noat
	LOAD		AT, AST * RSIZE(k0)
	LOAD		k0, K0 * RSIZE(k0)
	.set 		at

	.set		push
	.set		mips3
	eret
	.set		pop
END(__go)

/**
 * void __exit(void)
 * 描述：客户程序返回地址，该函数将跳转至go_return_jmpbuf对应的setjmp位置
 *		 ，并将客户程序返回值保存在go_return_val中，该函数永不返回
 * 使用寄存器：a0, a1, v0
 */
LEAF(__exit)	
	mfc0		a0, CP0_SR
	li			a1, SR_EXL
	not 		a1, a1
	and			a0, a1
	mtc0		a0, CP0_SR
	
	sw			v0, go_return_val
	la			a0, go_return_jmpbuf
	li			a1, 1
	bal			longjmp
	nop
END(__exit)

/**
 * unsigned long get_cpu_count(void) 
 * 描述：读取MIPS CPU片内计数器
 * 使用寄存器：v0
 */
LEAF(get_cpu_count)		
	/* 这里只读低32位的值 */
	mfc0		v0, CP0_COUNT 
	nop
	jr			ra
	nop
END(get_cpu_count)

	/* 注意：使用.common定义的变量可以不带标号 */
.data

	/* 客户程序返回值 */
	.global 	go_return_val
go_return_val:
	.word		0x0

	/* go执行命令使用jmp_buf变量存储位置 */
	.global 	go_return_jmpbuf
	.align		3
	.common		go_return_jmpbuf, 12 * 8

	/* bootcore_regsframe变量存储位置 */
	.global 	bootcore_regsframe
	.align		3
	.common		bootcore_regsframe, 128 * 8

	.set		reorder
