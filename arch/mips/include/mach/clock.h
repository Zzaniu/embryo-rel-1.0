
/**
 * Copyright(c) 2018-10-22 @Shangwen Wu	
 *
 * 依赖架构相关的时钟定义
 * 
 */
#ifndef __MACH_CLOCK_H__
#define __MACH_CLOCK_H__

extern void mach_clkinit(void); //defined in clock.c
extern void mach_udelay(unsigned long usec);

#endif	//__MACH_CLOCK_H__
