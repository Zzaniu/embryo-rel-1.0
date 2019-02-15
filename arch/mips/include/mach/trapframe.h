
/**
 * Copyright(c) 2019-1-26 Shangwen Wu @PMON
 *
 * CPU核寄存器缓存表定义（架构相关） 
 * 
 */

#ifndef __MACH_TRAPFRMAME_H__
#define	__MACH_TRAPFRMAME_H__

#ifdef _LOCORE

/* trapframe需要进行保存的几个寄存器在trapframe结构中的偏移 */
#define ZERO	0
#define AST		1
#define V0		2
#define V1		3
#define A0		4
#define A1		5
#define A2		6
#define A3		7
#define T0		8
#define T1		9
#define T2		10
#define T3		11
#define T4		12
#define T5		13
#define T6		14
#define T7		15
#define S0		16
#define S1		17
#define S2		18
#define S3		19
#define S4		20
#define S5		21
#define S6		22
#define S7		23
#define T8		24
#define T9		25
#define K0		26
#define K1		27
#define GP		28
#define SP		29
#define S8		30
#define RA		31
#define MULLO	32
#define MULHI	33
#define BADVADDR	34
#define CAUSE	35
#define EPC		36
#define SR		37

/* 以下寄存器不在trapframe中进行保存 */
#define NUMSAVEREGS	38
#define FPBASE		NUMSAVEREGS

#define F0		(FPBASE + 0) 
#define F1		(FPBASE + 1)
#define F2		(FPBASE + 2)
#define F3		(FPBASE + 3)
#define F4		(FPBASE + 4)
#define F5		(FPBASE + 5)
#define F6		(FPBASE + 6)
#define F7		(FPBASE + 7)
#define F8		(FPBASE + 8)
#define F9		(FPBASE + 9)
#define F10		(FPBASE + 10) 
#define F11		(FPBASE + 11)
#define F12		(FPBASE + 12)
#define F13		(FPBASE + 13)
#define F14		(FPBASE + 14)
#define F15		(FPBASE + 15)
#define F16		(FPBASE + 16)
#define F17		(FPBASE + 17)
#define F18		(FPBASE + 18)
#define F19		(FPBASE + 19)
#define F20		(FPBASE + 20)
#define F21		(FPBASE + 21)
#define F22		(FPBASE + 22)
#define F23		(FPBASE + 23)
#define F24		(FPBASE + 24)
#define F25		(FPBASE + 25)
#define F26		(FPBASE + 26)
#define F27		(FPBASE + 27)
#define F28		(FPBASE + 28)
#define F29		(FPBASE + 29)
#define F30		(FPBASE + 30)
#define F31		(FPBASE + 31)
#define FSR		(FPBASE + 32)

#define NUMFPREGS	33

#define CP0BASE	(FPBASE + NUMFPREGS)

#define COUNT	(CP0BASE + 0)		
#define COMPARE	(CP0BASE + 1)
#define WATCHLO	(CP0BASE + 2) 
#define WATCHHI	(CP0BASE + 3)
#define WATCHM	(CP0BASE + 4) 
#define WATCH1	(CP0BASE + 5)
#define WATCH2	(CP0BASE + 6)
#define LLADR	(CP0BASE + 7)
#define ECC		(CP0BASE + 8)
#define CACHER	(CP0BASE + 9)
#define TAGLO	(CP0BASE + 10) 
#define TAGHI	(CP0BASE + 11)
#define WIRED	(CP0BASE + 12)
#define PGMSK	(CP0BASE + 13)
#define ENTLO0	(CP0BASE + 14) 
#define ENTLO1	(CP0BASE + 15)
#define ENTHI	(CP0BASE + 16)
#define CONTEXT (CP0BASE + 17)
#define XCONTEXT	(CP0BASE + 18)
#define INDEX	(CP0BASE + 19)		
#define RANDOM	(CP0BASE + 20)
#define CONFIG	(CP0BASE + 21)
#define ICR		(CP0BASE + 22)
#define IPLLO	(CP0BASE + 23) 
#define IPLHI	(CP0BASE + 24)
#define PRID	(CP0BASE + 25)
#define PCOUNT	(CP0BASE + 26)
#define PCTRL	(CP0BASE + 27)
#define ERRPC	(CP0BASE + 28)

#define NUMCP0REGS	29

#else

#include <sys/types.h>

/**
 * 对CPU和寄存器的C变量描述，C函数将期望写入的值填入该结构中
 * ，而该结构变量最终将在__go异常处理中被更新到CPU核寄存器 
 */
struct trapframe {
	/* 通用寄存器组 */
	register_t zero;
	register_t ast;
	register_t v0;
	register_t v1;
	register_t a0;
	register_t a1;
	register_t a2;
	register_t a3;
	register_t t0;
	register_t t1;
	register_t t2;
	register_t t3;
	register_t t4;
	register_t t5;
	register_t t6;
	register_t t7;
	register_t s0;
	register_t s1;
	register_t s2;
	register_t s3;
	register_t s4;
	register_t s5;
	register_t s6;
	register_t s7;
	register_t t8;
	register_t t9;
	register_t k0;
	register_t k1;
	register_t gp;
	register_t sp;
	register_t s8;
	register_t ra;
	/* trapframe需要进行保存几个CP0寄存器 */
	register_t mullo;
	register_t mulhi;
	register_t badvaddr;
	register_t cause;
	register_t epc;
	register_t sr;

	/* 浮点运算相关寄存器 */
	f_register_t f0;
	f_register_t f1;
	f_register_t f2;
	f_register_t f3;
	f_register_t f4;
	f_register_t f5;
	f_register_t f6;
	f_register_t f7;
	f_register_t f8;
	f_register_t f9;
	f_register_t f10;
	f_register_t f11;
	f_register_t f12;
	f_register_t f13;
	f_register_t f14;
	f_register_t f15;
	f_register_t f16;
	f_register_t f17;
	f_register_t f18;
	f_register_t f19;
	f_register_t f20;
	f_register_t f21;
	f_register_t f22;
	f_register_t f23;
	f_register_t f24;
	f_register_t f25;
	f_register_t f26;
	f_register_t f27;
	f_register_t f28;
	f_register_t f29;
	f_register_t f30;
	f_register_t f31;
	register_t fsr;

	/* 协处理器0相关寄存器 */
	register_t count;
	register_t compare;
	register_t watchlo;
	register_t watchhi;
	register_t watchm;
	register_t watch1;
	register_t watch2;
	register_t lladr;
	register_t ecc;
	register_t cacher;
	register_t taglo;
	register_t taghi;
	register_t wired;
	register_t pgmsk;
	register_t entlo0;
	register_t entlo1;
	register_t enthi;
	register_t context;
	register_t xcontext;
	register_t index;
	register_t random;
	register_t config;
	register_t icr;
	register_t ipllo;
	register_t iplhi;
	register_t prid;
	register_t pcount;
	register_t pctrl;
	register_t errpc;
};
#endif

#endif	//__MACH_TRAPFRMAME_H__ 
