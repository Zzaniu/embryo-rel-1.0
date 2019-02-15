
/**
 * Copyright(c) 2016-8-9 Shangwen Wu	
 *
 * 系统内存地址空间分配
 * 
 */

#ifndef __MACH_BONITO_H__
#define __MACH_BONITO_H__

/* boot flash地址定义 */
#define NVRAM_BASE				0x1fc00000
#define NVRAM_SIZE				0x100000			//当前BOOTflash的大小为1MB
#define NVRAM_ENVOFFS			0x80000				//flash保存环境变量的偏移地址，这里要求前面的代码必须小于512K
#define NVRAM_ENVSIZE			(256*64+2)			//当前BIOS最大支持的环境变量个数为64个，每个变量名和变量值最大占256B

/* 显存地址空间定义 */
#define FB_BASE					0x0e800000			/* FB0显存空间物理基地址 */
#define CURSOR_BASE				0x0eff0000			/* 光标显存空间物理基地址 */

#endif //__MACH_BONITO_H__

