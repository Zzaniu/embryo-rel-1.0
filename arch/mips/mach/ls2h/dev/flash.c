
/**
 * Copyright(c) 2016-8-9 Shangwen Wu	
 *
 * LS2H平台flash相关操作
 * 
 */
#include <common.h>
#include <flash/flash_map.h>
#include <mach/types.h>
#include <asm/cpu.h>
#include <mach/pflash.h>

static struct flash_map plat_flashmaps[] = LS2H_PLAT_FLASH_MAP;

/**
 * 描述：根据基地值查找对应的MAP映射
 */
struct flash_map *find_flash_map(unsigned long base)
{
	struct flash_map *p = NULL;
		
	for(p = plat_flashmaps; p->flm_name != NULL; ++p)
		if(base >= p->flm_base && base < p->flm_base + p->flm_size)
			return p;

	return NULL;
}

