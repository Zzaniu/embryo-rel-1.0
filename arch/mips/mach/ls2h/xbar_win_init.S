
/**
 * Copyright(c) 2015-7-16 Shangwen Wu	
 *
 * 初始化系统映射窗口，增加系统稳定性
 * 
 */

#define XBAR_WIN_REG_BASE			0xbfd80000
#define XBAR_WIN_REG_BASE_M6		0xbfd82600
#define XBAR_WIN_BASE_OFFSET		0x00
#define XBAR_WIN_MASK_OFFSET		0x40
#define XBAR_WIN_MMAP_OFFSET		0x80

/* 设置指定窗口的值 */
#ifndef XBAR_WIN_CONFIG
#define XBAR_WIN_CONFIG(mastersel, winsel, base, mask, mmap)				\
			li			a0, XBAR_WIN_REG_BASE+mastersel*0x100+winsel*0x8;	\
			dli			a1, base;											\
			sd			a1, XBAR_WIN_BASE_OFFSET(a0);						\
			dli			a1, mask;											\
			sd			a1, XBAR_WIN_MASK_OFFSET(a0);						\
			dli			a1, mmap;											\
			sd			a1, XBAR_WIN_MMAP_OFFSET(a0)
#endif

#ifndef XBAR_WIN_CONFIG_M6				
#define XBAR_WIN_CONFIG_M6(winsel, base, mask, mmap)						\
			li			a0, XBAR_WIN_REG_BASE_M6+winsel*0x8;				\
			dli			a1, base;											\
			sd			a1, XBAR_WIN_BASE_OFFSET(a0);						\
			dli			a1, mask;											\
			sd			a1, XBAR_WIN_MASK_OFFSET(a0);						\
			dli			a1, mmap;											\
			sd			a1, XBAR_WIN_MMAP_OFFSET(a0)
#endif

#ifndef XBAR_WIN_CLEAR
#define XBAR_WIN_CLEAR(mastersel, winsel)									\
			li			a0, XBAR_WIN_REG_BASE+mastersel*0x100+winsel*0x8;	\
			sd			zero, XBAR_WIN_BASE_OFFSET(a0);						\
			sd			zero, XBAR_WIN_MASK_OFFSET(a0);						\
			sd			zero, XBAR_WIN_MMAP_OFFSET(a0)
#endif

#ifndef XBAR_WIN_CLEAR_M6
#define XBAR_WIN_CLEAR_M6(mastersel, winsel)								\
			li			a0, XBAR_WIN_REG_BASE_M6+winsel*0x8;				\
			sd			zero, XBAR_WIN_BASE_OFFSET(a0);						\
			sd			zero, XBAR_WIN_MASK_OFFSET(a0);						\
			sd			zero, XBAR_WIN_MMAP_OFFSET(a0)
#endif


	/* Copyright @loongson */
	li		a0, 0xbfd800a8						/* 尽量将将设置与PMON保持一致，虽然不知道在干什么 */
	dli		t1, 0x40000084
	sd		t1, 0x00(a0)

	li		a0, 0xbfd80090				
	dli		t1, 0x00000082
	sd		t1, 0x00(a0)

	/* sbrige */
	XBAR_WIN_CONFIG(1, 0, 0x0000000000000000, 0xfffffffff0000000, 0x00000000400000f3)		//lower 256M
	XBAR_WIN_CONFIG(1, 1, 0x0000000000000000, 0xffffffffc0000000, 0x00000001400000f3)		//0~1G
	XBAR_WIN_CONFIG(1, 2, 0x0000000040000000, 0xffffffffc0000000, 0x00000002400000f3)		//1G~2G
	XBAR_WIN_CONFIG(1, 3, 0x0000000080000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G
	XBAR_WIN_CONFIG(1, 4, 0x00000000c0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G
	XBAR_WIN_CONFIG(1, 5, 0xffffffff80000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G for sata/usb XKPHY
	XBAR_WIN_CONFIG(1, 6, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G for sata/usb XKPHY

	/* pcie */
	XBAR_WIN_CONFIG(4, 0, 0x0000000000000000, 0xfffffffffffff000, 0x000000001fd00081)		//lower 4k
	XBAR_WIN_CONFIG(4, 1, 0x0000000000000000, 0xfffffffff0000000, 0x00000000400000f3)		//lower 256M
	XBAR_WIN_CONFIG(4, 2, 0x0000000000000000, 0xffffffffc0000000, 0x00000001400000f3)		//0~1G
	XBAR_WIN_CONFIG(4, 3, 0x0000000040000000, 0xffffffffc0000000, 0x00000002400000f3)		//1G~2G
	XBAR_WIN_CONFIG(4, 4, 0x0000000080000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G
	XBAR_WIN_CONFIG(4, 5, 0x00000000c0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G
	XBAR_WIN_CONFIG(4, 6, 0xffffffff80000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G for sata/usb XKPHY
	XBAR_WIN_CONFIG(4, 7, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G for sata/usb XKPHY

	/* gpu */
	XBAR_WIN_CONFIG(2, 0, 0x0000000000000000, 0xfffffffff0000000, 0x00000000400000f3)		//lower 256M
	XBAR_WIN_CONFIG(2, 1, 0x0000000000000000, 0xffffffffc0000000, 0x00000001400000f3)		//0~1G
	XBAR_WIN_CONFIG(2, 2, 0x0000000040000000, 0xffffffffc0000000, 0x00000002400000f3)		//1G~2G
	XBAR_WIN_CONFIG(2, 3, 0x0000000080000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G
	XBAR_WIN_CONFIG(2, 4, 0x00000000c0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G
	XBAR_WIN_CONFIG(2, 5, 0xffffffff80000000, 0xffffffffc0000000, 0x00000003400000f3)		//2G~3G for sata/usb XKPHY
	XBAR_WIN_CONFIG(2, 6, 0xffffffffc0000000, 0xffffffffc0000000, 0x00000004400000f3)		//3G~4G for sata/usb XKPHY

	/* unkown */
	XBAR_WIN_CONFIG_M6(0, 0x0000000040000000, 0xffffffffc0000000, 0x00000000000000f0)		
	XBAR_WIN_CONFIG_M6(1, 0x0000000140000000, 0xffffffffc0000000, 0x00000001000000f0)		
	XBAR_WIN_CONFIG_M6(2, 0x0000000240000000, 0xffffffffc0000000, 0x00000001400000f0)		
	XBAR_WIN_CONFIG_M6(3, 0x0000000340000000, 0xffffffffc0000000, 0x00000001800000f0)		
	XBAR_WIN_CONFIG_M6(4, 0x0000000440000000, 0xffffffffc0000000, 0x00000001c00000f0)		
	XBAR_WIN_CONFIG_M6(5, 0x0000000000000000, 0x0000000000000000, 0x00000000000000f0)	

