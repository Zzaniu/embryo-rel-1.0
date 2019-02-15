
/**
 * Copyright(c) 2017-1-19 Shangwen Wu
 * stmmac(gmac)硬件操作
 * 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <sys/list.h>
#include <sys/system.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <fs/termio.h>

#include "stmmac.h"

uint32_t stmmac_read_reg(struct stmmac_device *stmdev, uint32_t reg)
{
	uint64_t addr = stmdev->iobase + (uint64_t)reg;
	uint32_t val;

#if __mips >= 3 && __mips != 32	
	__asm __volatile(
		".set	noreorder\n"
		".set	mips3\n"
		"ld		$8, %1\n"
		"lw		$9, 0x0($8)\n"
		"nop\n"
		"nop\n"
		"sw		$9, %0\n"
		".set	reorder\n"
		".set	mips0\n"
		:"=m"(val)
		:"m"(addr)
		:"memory", "$8", "$9"
	);
#else
	val = *(volatile uint32_t *)addr;
#endif

	return val;
}

void stmmac_write_reg(struct stmmac_device *stmdev, uint32_t reg, uint32_t val)
{
	uint64_t addr = stmdev->iobase + (uint64_t)reg;

#if __mips >= 3 && __mips != 32	
	__asm __volatile(
		".set	noreorder\n"
		".set	mips3\n"
		"lw		$9, %0\n"
		"ld		$8, %1\n"
		"sw		$9, 0x0($8)\n"
		".set	reorder\n"
		".set	mips0\n"
		:
		:"m"(val), "m"(addr)
		:"memory", "$8", "$9"
	);
#else
	*(volatile uint32_t *)addr = val;
#endif
}

