
/**
 * Copyright(c) 2019-1-25 Shangwen Wu	
 *
 * setjmp定义（架构相关） 
 * 
 */

#ifndef __MACH_SETJMP_H__
#define __MACH_SETJMP_H__

/* 对于MIPS架构而言，jmp_buf需要保存s0-s8(fp)、sp、gp、ra这个几个寄存器 */
#define __JBLEN		(sizeof(register_t) * 12)

#endif //__MACH_SETJMP_H__
