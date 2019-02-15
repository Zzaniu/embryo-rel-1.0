
/**
 * Copyright(c) 2015-6-6 Shangwen Wu	
 *
 * 汇编公共相关头文件
 * 
 */

#ifndef __ASM_H__
#define __ASM_H__

/* 汇编ABI定义 */
#define FRAME_STAND_SIZE			24		//一个函数的标准栈空间大小：16字节参数寄存器预留空间+4个字节的返回地址寄存器+4个字节的返回地址对齐pad
#define FRAME_ARG_SIZE				16		//为a0-a3四个参数寄存集预留的栈空间
#define FRAME_RA_OFFS				20		//保存返回地址寄存器的偏移

/* 用于定义简单的函数，不调用其他函数，且局部变量大多只用到a0-a3、v0-v1 */
#define LEAF(name)					\
		.text;						\
		.global		name;			\
		.ent		name;			\
name:

/* 用于定义非叶子函数，函数包含子调用，函数内部包含出入栈操作 */
#define NON_LEAF(name, fsize, retpc) \
		.text;						\
		.global		name;			\
		.ent		name;			\
name:;								\
		.frame 		sp, fsize, retpc	

#define END(name)					\
		.size 		name, .-name;	\
		.end		name
		
#if __mips < 3
	#define LOAD	lw
	#define STORE	sw
	#define RSIZE	4
	#define MTC0	mtc0
	#define MFC0 	mfc0
#else
	#define LOAD	ld
	#define STORE	sd
	#define RSIZE	8
	#define MTC0	dmtc0
	#define MFC0 	dmfc0
#endif

#endif /* __ASM_H__ */

