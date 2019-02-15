
/**
 * Copyright(c) 2016-8-9 Shangwen Wu	
 *
 * flash地址映射相关头文件
 * 
 */
#ifndef __FLASH_MAP_H__
#define __FLASH_MAP_H__

#define FL_BUS_BIT_WIDTH_8		8
#define FL_BUS_BIT_WIDTH_16		16

struct flash_map {
	char *flm_name;
	unsigned long flm_base;
	unsigned long flm_size;
	unsigned int flm_width;
};

#endif //__FLASH_MAP_H__

