
/**
 * Copyright(c) 2019-1-25 Shangwen Wu	
 *
 * setjmp定义 
 * 
 */

#ifndef __SETJMP_H__
#define __SETJMP_H__

#include <mach/types.h>
#include <mach/setjmp.h>

/* 用于保存栈信息的缓冲区，其缓冲长度与架构相关 */
#if __mips >= 3 && __mips != 32	
typedef register_t jmp_buf[__JBLEN] __attribute__((aligned(8)));
#else
typedef register_t jmp_buf[__JBLEN] __attribute__((aligned(4)));
#endif

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

#endif //__SETJMP_H__
