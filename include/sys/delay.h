

/**
 * Copyright(c) 2018-10-22 Shangwen Wu	
 *
 * delay延时接口
 * 
 */

#ifndef __SYS_DELAY_H__
#define __SYS_DELAY_H__

#include <mach/clock.h>

#define udelay mach_udelay

#define mdelay(ms) udelay((ms) * 1000)				//毫秒级延时	
#define delay(s) udelay((s) * 1000000)				//秒级延时

#endif //__SYS_DELAY_H__

