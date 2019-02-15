
/**
 * Copyright(c) 2015-6-16 Shangwen Wu	
 *
 * LS2H DDR3控制器初始化（仅考虑MC0） 
 * 
 * 寄存器使用说明：
 *	s1：包含DDR的所有配置信息
 *  s2: 保存内存大小
 *  s3: leveled mark
 */

/***************************************************** 
 * 1. 计算S0配置中内存大小，不合法的DDR大小将报错退出*
 ****************************************************/
	MC0_GET_INFO_TO_a1(MEMORY_SIZE)
	beqz		a1, 88f
	nop
	dli			t0, 0x10
	bgt			a1, t0, 88f					/* 不支持超过16*512MB大小的内存 */
	nop
	move		s2, a1

/*********************************
 * 2. 判断当前初始化是否需要level*
 ********************************/
	bal			check_need_level	
	nop
	bnez		v0, 1f
	move		s3, v0
	PRINTSTR("SDRAM controller is leveled\r\n")
	b			2f
	nop
1:
	PRINTSTR("SDRAM controller needs leveling, wait for a moment...\r\n")
2:

/*********************************
 * 3. 调用mc_init初始化内存控制器*
 ********************************/
	MC0_GET_INFO_TO_a1(SDRAM_TYPE)
	dli			a0, 0x03
	beq			a1, a0, 1f
	nop
	PRINTSTR("DDR2 controller init... ")
1:
	PRINTSTR("DDR3 controller init... ")

	bal			mc_init
	nop	
	bnez		v0, 1f
	nop
	PRINTSTR("OK!\r\n")
	b			ddr_init_end
	nop
1:
	PRINTSTR("fail!!!\r\n")
	b			ddr_init_end
	nop

88:
	PRINTSTR("memory size unsupport! DDR init failed!\r\n");

ddr_init_end:

