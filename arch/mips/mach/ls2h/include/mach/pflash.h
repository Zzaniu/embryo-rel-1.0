
/**
 * Copyright(c) 2016-8-9 Shangwen Wu	
 *
 * LS2H平台flash相关头文件
 * 
 */
#ifndef __MACH_PFLASH_H__
#define __MACH_PFLASH_H__

#define get_nvram_base()	(0xbfc00000)			//LS2H的NORFLASH起始地址

#define LS2H_PLAT_FLASH_MAP	{{"sst25vf032b", PHY_TO_UNCACHED(0x1fc00000), 0x100000, FL_BUS_BIT_WIDTH_8}, {}}

#endif //__MACH_PFLASH_H__
