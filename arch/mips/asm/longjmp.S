
/**
 * Copyright(c) 2016-2-4 Shangwen Wu	
 *
 * 长跳转接口实现
 * 
 */
#include <asm/regdef.h>
#include <asm/asm.h>

	/* 注意：必须关闭编译器重新排序指令 */
	.set 		noreorder

/**
 * int setjmp(jmp_buf env);
 * 描述：setjmp接口实现
 * 使用寄存器：v0, a0
 * 注意：长跳转后，仅对s0-s8(fp)、sp、gp、ra这个几个寄存器进行回滚，应避免在调
 *		 用longjmp后使用setjmp函数之前的上下文环境
 * 返回：当直接调用该函数时返回0，当从longjmp跳转时，返回longjmp设置的val值
 */
LEAF(setjmp)		
	STORE	 	s0, 0x0 * RSIZE(a0)
	STORE	 	s1, 0x1 * RSIZE(a0)
	STORE	 	s2, 0x2 * RSIZE(a0)
	STORE	 	s3, 0x3 * RSIZE(a0)
	STORE	 	s4, 0x4 * RSIZE(a0)
	STORE	 	s5, 0x5 * RSIZE(a0)
	STORE	 	s6, 0x6 * RSIZE(a0)
	STORE	 	s7, 0x7 * RSIZE(a0)
	STORE	 	fp, 0x8 * RSIZE(a0)
	STORE	 	sp, 0x9 * RSIZE(a0)
	STORE	 	gp, 0xa * RSIZE(a0)
	STORE	 	ra, 0xb * RSIZE(a0)
	/* 直接调用setjmp将永远返回0 */
	jr			ra
	move		v0, zero
END(setjmp)

/**
 * void longjmp(jmp_buf env, int val);
 * 描述：longjmp接口实现
 * 使用寄存器：v0, a0, a1
 * 
 */
LEAF(longjmp)		
	/* 这里将ra加载提前，以防止jr ra指令产生多余的等待周期 */
	LOAD	 	ra, 0xb * RSIZE(a0)
	LOAD	 	s0, 0x0 * RSIZE(a0)
	LOAD	 	s1, 0x1 * RSIZE(a0)
	LOAD	 	s2, 0x2 * RSIZE(a0)
	LOAD	 	s3, 0x3 * RSIZE(a0)
	LOAD	 	s4, 0x4 * RSIZE(a0)
	LOAD	 	s5, 0x5 * RSIZE(a0)
	LOAD	 	s6, 0x6 * RSIZE(a0)
	LOAD	 	s7, 0x7 * RSIZE(a0)
	LOAD	 	fp, 0x8 * RSIZE(a0)
	LOAD	 	sp, 0x9 * RSIZE(a0)
	LOAD	 	gp, 0xa * RSIZE(a0)
	/* 这里使用setjmp函数的返回地址，并且使用val作为返回值 */
	jr			ra
	move		v0, a1
END(longjmp)

	.set		reorder
