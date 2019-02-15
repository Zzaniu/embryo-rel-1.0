
/**
 * Copyright(c) 2015-6-5 Shangwen Wu	
 *
 * MIPS架构的寄存器定义 
 * 
 */
#ifndef __REGDEF_H__
#define	__REGDEF_H__

/* 通用寄存器定义 */
#define zero		$0					/* 永远返回0 */

#define AT			$1					/* 汇编器暂存寄存器 */

#define v0			$2					/* 子程序返回值 */
#define v1			$3					/* 子程序返回值 */

#define a0			$4					/* 函数调用时，存放参数 */
#define a1			$5
#define a2			$6
#define a3			$7

#define t0			$8					/* 临时变量，子程序不会对该变量进行保存 */
#define t1			$9
#define t2			$10
#define t3			$11
#define t4			$12
#define t5			$13
#define t6			$14
#define t7			$15
	
#define s0			$16					/* 子程序寄存器，子程序负责回复和保存该值 */
#define s1			$17
#define s2 			$18
#define s3			$19
#define s4			$20
#define s5			$21
#define s6			$22
#define s7			$23

#define t8			$24					/* 临时变量，子程序不会对该变量进行保存 */
#define t9 			$25

#define	k0			$26					/* 中断以及自陷处理程序中使用 */
#define k1 			$27				

#define gp			$28					/* 全局指针，用于指向static和extern变量 */

#define sp			$29					/* 堆栈指针寄存器 */

#define fp			$30					/* 一般用于栈帧指针寄存器 */
#define s8			fp
	
#define ra			$31					/* 子程序返回地址寄存器 */

#endif /* __REGDEF_H__ */

