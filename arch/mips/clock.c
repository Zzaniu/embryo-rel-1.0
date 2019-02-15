
/**
 * Copyright(c) 2018-10-22 Shangwen Wu	
 *
 * mips架构的时钟相关代码 
 * 
 */

#include <mach/types.h>
#include <mach/limits.h>
#include <mach/clock.h>

#ifdef HAVE_QUAD
#define DELAY_USE_LONGLONG 0
#endif

extern unsigned long probe_cpu_pipe_frequency(void); //defined post.c
extern unsigned long get_cpu_count(void); //defined in mips.S

static uint32_t clkperusec = 0;			//系统每微秒节拍数

/**
 * 描述：系统时间初始化 
 */
void mach_clkinit(void)
{
	clkperusec = probe_cpu_pipe_frequency() / 1000000;
}

/**
 * 描述：微秒级延时函数
 */
void mach_udelay(unsigned long usec)
{
#if DELAY_USE_LONGLONG
	register uint64_t c1, c2, n;
#else
	register unsigned long c1, c2, n;
#endif

	c1 = get_cpu_count();
	c2 = get_cpu_count();
	
	//正常情况这里c2将比c1大6-7左右
	if(c1 != c2) {
#if DELAY_USE_LONGLONG
		n = (uint64_t)usec * ((uint64_t)clkperusec / 2);	//注意CPU COUNT计数频率为主频的一半
#else
		n = usec * (clkperusec / 2);		//注意CPU COUNT计数频率为主频的一半
#endif
		while(1) {
			if((c2 = get_cpu_count()) > c1) {
				if(c2 - c1 >= n)
					return;
			} else if(ULONG_MAX - (c1 - c2) >= n)
				return;
		}
	} else {
		//当CPU-COUNT停止工作时，假设一条访存指令为1us
		while(usec--)
			*(volatile unsigned char *)0xbfc00000;
	}	
}

