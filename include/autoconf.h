
/**
 * Copyright(c) 2015-6-6 Shangwen Wu	
 *
 * boot配置头文件，此文件将动态生成
 * 
 */

#ifndef __AUTOCONF_H__
#define __AUTOCONF_H__

#include <rgb.h>

#define CPUFREQ						800				/* 设置CPU主频为800M */
#define DDRFREQ						300				/* DDR时钟为300M */
#define BIOS_CONSOLE_UART			UART0			/* BIOS控制串口，当选择uart0以外的串口时，必须设置UART_SPLIT_ENABLE */
#define UART_SPLIT_ENABLE			1				/* 使能多串口复用 */
#define BIOS_I2C_CONTROLLER			I2C0			/* BIOS-I2C控制器 */	
#define CONSOLE_UART_BAUDRATE		115200			/* 串口波特率配置 */
#define FPU_ENABLE					1				/* 时能浮点处理单元 */
#define POWERON_SELF_TEST			1				/* 是否进行上电自检测 */
#define USE_SHELL_CONSOLE			1				/* 使用shell终端 */
#define	SOFT_64BIT_DIV_MOD			1				/* 是否使用软件64位除法和求模运算 */
#define USE_DC_VGA					1				/* 是否使用2H的VGA作为显示控制台，否则将使用DVO */
#define VIDEO_1440X900				1				/* 显示分辨率 */
#define VIDEO_32BPP					1				/* 每个像素占用位宽*/
//#define VIDEO_800X600				1				/* 显示分辨率 */
//#define VIDEO_16BPP				1				/* 每个像素占用位宽*/
#define OUTPUT_TO_BOTH				1				/* 从串口和显示器输出调试信息 */

#ifdef VIDEO_1BPP
	#define FB_CONSOLE_BG				0x00			/* 显示控制台背景色 */
	#define FB_CONSOLE_FG				0x01			/* 显示控制台前景色 */
#elif VIDEO_2BPP
	#define FB_CONSOLE_BG				0x00
	#define FB_CONSOLE_FG				0x03	
#elif VIDEO_4BPP
	#define FB_CONSOLE_BG				0x00
	#define FB_CONSOLE_FG				0x0f
#elif VIDEO_8BPP										/* RGB332 */ 
	#define FB_CONSOLE_BG				RGB332(0, 0, 0x3)
	#define FB_CONSOLE_FG				RGB332(7, 7, 3)
#elif VIDEO_15BPP										/* RGB555 */ 
	#define FB_CONSOLE_BG				RGB555(0, 0, 0x1f)
	#define FB_CONSOLE_FG				RGB555(0x1f, 0x1f, 0x1f)	
#elif VIDEO_16BPP										/* RGB565 */ 
	#define FB_CONSOLE_BG				RGB565(0, 0, 0x1f)
	#define FB_CONSOLE_FG				RGB565(0x1f, 0x3f, 0x1f)
#elif VIDEO_24BPP										/* RGB888 */ 
	#define FB_CONSOLE_BG				RGB888(0, 0, 0xff)
	#define FB_CONSOLE_FG				RGB888(0xff, 0xff, 0xff)
#elif VIDEO_32BPP										/* RGBX888 */ 
	#define FB_CONSOLE_BG				RGBX888(0, 0, 0xff)
	#define FB_CONSOLE_FG				RGBX888(0xff, 0xff, 0xff)
#else
	#define FB_CONSOLE_BG				0xff
	#define FB_CONSOLE_FG				0xff
#endif

#define FIX_HEAP_SPACE				1				/* malloc使用固定地址的堆空间 */
#define HEAP_BASE					0x8f400000		/* bios堆起始地址，当FIX_HEAP_SPACE为1时有效 */
#define HEAP_SIZE					0x400000		/* bios堆大小，当FIX_HEAP_SPACE为1时有效 */

#endif /* __AUTOCONF_H__  */

