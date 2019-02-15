
/**
 * Copyright(c) 2015-7-21 Shangwen Wu	
 *
 * LS2H DDR内存测试（仅考虑MC0） 
 * 
 */

#include "mem_test.h"

//#define DEBUG_MEM_TEST								//调试开关
#ifdef DEBUG_MEM_TEST
#define MT_DEBUG(str)						\
	.rdata; 101: .asciz str; .text; la a0, 101b; bal serial_puts; nop
#else
#define MT_DEBUG(str)	;
#endif

/*
 * 描述：内存测试函数
 * 参数：
 * 寄存器使用：t9返回地址，s6：内存测试上界地址，s5：内存测试下界地址, s2、s3: 临时返回值
 * t5：当前访问的内存地址，t6：当前访问的测试数据地址，s7：剩余片选MAP值，s4：当前测数数据
 * 组号，t7：调试用，显示错误条目数，t9：返回地址
 * 返回：v0保存所有字节测试结果，v1保存错误字节重复读取结果
 *
 */
	.global	mem_test
	.ent	mem_test
mem_test:
	/* 保存局部变量 */
	GET_TM_STACK_BASE_TO_a0
	sd			t0, 0x00(a0)
	sd			t1, 0x08(a0)
	sd			t2, 0x10(a0)
	sd			t3, 0x18(a0)
	sd			t4, 0x20(a0)
	sd			t5, 0x28(a0)
	sd			t6, 0x30(a0)
	sd			t7, 0x38(a0)
	sd			t8, 0x40(a0)
	sd			t9, 0x48(a0)
	sd			s0, 0x50(a0)
	sd			s1, 0x58(a0)
	sd			s2, 0x60(a0)
	sd			s3, 0x68(a0)
	sd			s4, 0x70(a0)
	sd			s5, 0x78(a0)
	sd			s6, 0x80(a0)
	sd			s7, 0x88(a0)

	move		t9, ra

	MT_DEBUG("start memory test...\r\n")
		
	dli			s2, 0x0								/* 临时保存测试结果 */	
	dli			s3, 0x0

	MC0_GET_INFO_TO_a1(MC_CS_MAP)		
	move		s7, a1
	/* BUG：以下代码并未对实际的片选进行操作，这里假设片选只有一个 */
mem_test_begin:
	beqz		s7, mem_test_end
	nop
1:													/* 检查当前片选是否选中，未选中的将被跳过 */
	and			a0, s7, 0x01
	beqz		a0, 1b 
	dsrl		s7, 1
	
	/* 计算内存大小 */	
	dli			a2, ROW_PINS_MAX					//a2 = col_pins + row_pins + bank_pins
	MC0_GET_INFO_TO_a1(SDRAM_ADDR_PINS)		
	dsubu		a2, a1
	daddu		a2, COL_PINS_MAX
	MC0_GET_INFO_TO_a1(SDRAM_COL_SIZE)		
	daddu		a1, 2								//bad code
	dsubu		a2, a1
	MC0_GET_INFO_TO_a1(SDRAM_EIGHT_BANK)		
	beqz		a1, 1f
	daddiu		a2, a2, 2
	daddu		a2, 1						
1:													//DIMM=32:*4 DIMM=64:*8，==》芯片数*SDRAM位宽/8
	MC0_GET_INFO_TO_a1(DIMM_WIDTH)		
	bnez		a1, 1f
	daddiu		a2, a2, 2						
	daddu		a2, 1						
1:
	dli			a1, 0x01
	dsll		a0, a1, a2							//计算出内存大小
	GET_TM_MEM_BASE_TO_s6	
	daddu		s6, a0								//计算内存测试上界
	dsubu		s5, s6, TM_MEM_LIMIT				//计算内存测试下界	
	
#ifdef DEBUG_MEM_TEST
	MT_DEBUG("Test memory address range: 0x")
	dsrl		a0, s5, 32
	bal			serial_puthex
	nop
	move		a0, s5
	bal			serial_puthex
	nop
	MT_DEBUG(" ~ 0x")
	dsrl		a0, s6, 32
	bal			serial_puthex
	nop
	move		a0, s6
	bal			serial_puthex
	nop
	MT_DEBUG("\r\n")
#endif

	/* 将测试数据保存到指定位置 */	
	GET_TM_PATTERN_BASE_TO_t6
	dli			a0, PATTERN_D8_0_0
	sd			a0, 0x00(t6)	
	dli			a0, PATTERN_D8_0_1
	sd			a0, 0x08(t6)	
	dli			a0, PATTERN_D8_0_2
	sd			a0, 0x10(t6)	
	dli			a0, PATTERN_D8_0_3
	sd			a0, 0x18(t6)	
	dli			a0, PATTERN_D8_0_4
	sd			a0, 0x20(t6)	
	dli			a0, PATTERN_D8_0_5
	sd			a0, 0x28(t6)	
	dli			a0, PATTERN_D8_0_6
	sd			a0, 0x30(t6)	
	dli			a0, PATTERN_D8_0_7
	sd			a0, 0x38(t6)	
	daddu		t6, 0x40	
	dli			a0, PATTERN_D8_1_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_D8_1_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_D8_1_2
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_D8_1_3
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_D8_1_4
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_D8_1_5
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_D8_1_6
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_D8_1_7
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_D8_2_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_D8_2_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_D8_2_2
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_D8_2_3
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_D8_2_4
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_D8_2_5
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_D8_2_6
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_D8_2_7
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_D8_3_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_D8_3_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_D8_3_2
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_D8_3_3
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_D8_3_4
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_D8_3_5
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_D8_3_6
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_D8_3_7
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_D8_4_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_D8_4_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_D8_4_2
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_D8_4_3
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_D8_4_4
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_D8_4_5
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_D8_4_6
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_D8_4_7
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_D8_5_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_D8_5_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_D8_5_2
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_D8_5_3
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_D8_5_4
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_D8_5_5
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_D8_5_6
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_D8_5_7
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_DB_0_0
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_DB_0_1
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_DB_1_0
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_DB_1_1
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_DB_2_0
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_DB_2_1
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_DB_3_0
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_DB_3_1
	sd			a0, 0x38(t6)
	daddu		t6, 0x40	
	dli			a0, PATTERN_JUSTA
	sd			a0, 0x00(t6)
	dli			a0, PATTERN_JUST5
	sd			a0, 0x08(t6)
	dli			a0, PATTERN_FiveA
	sd			a0, 0x10(t6)
	dli			a0, PATTERN_ZEROONE
	sd			a0, 0x18(t6)
	dli			a0, PATTERN_L8b10b
	sd			a0, 0x20(t6)
	dli			a0, PATTERN_S8b10b
	sd			a0, 0x28(t6)
	dli			a0, PATTERN_Five7
	sd			a0, 0x30(t6)
	dli			a0, PATTERN_Zero2fd
	sd			a0, 0x38(t6)

	//init
	dli			s4, TEST_PATTERN_SIZE				/* 用于测试数据组循环控制 */
	GET_TM_PATTERN_BASE_TO_t6
	
next_pattern:
	move		t5, s5
	beqz		s4, no_pattern
	daddiu		s4, -1

#ifdef DEBUG_MEM_TEST
	MT_DEBUG("\r\nPattern group[")
	move		a0, s4
	bal			serial_puthex
	nop
	MT_DEBUG("] begin test...\r\n")

	dli			t7,	TM_MAX_ERRORS					/* 调试模式时，只打印指定数目的出错记录 */
#endif
	
	ld			a0, 0x00(t6)
	ld			a1, 0x08(t6)
	ld			a2, 0x10(t6)
	ld			a3, 0x18(t6)
	ld			t0, 0x20(t6)
	ld			t1, 0x28(t6)
	ld			v0, 0x30(t6)
	ld			v1, 0x38(t6)
1:
	beq			t5, s6, 1f
	nop
	sd			a0, 0x00(t5)
	sd			a1, 0x08(t5)
	sd			a2, 0x10(t5)
	sd			a3, 0x18(t5)
	sd			t0, 0x20(t5)
	sd			t1, 0x28(t5)
	sd			v0, 0x30(t5)
	sd			v1, 0x38(t5)

	cache		Hit_WriteBack_Inv_D, 0x00(t5)
	cache		Hit_WriteBack_Inv_D, 0x20(t5)
	cache		Hit_WriteBack_Inv_S, 0x00(t5)
	cache		Hit_WriteBack_Inv_S, 0x20(t5)

	b			1b
	daddiu		t5,	TM_MEM_INTERVAL 

1:													/* 开始进行比较 */	
	sync
	//wait a while，否则后面读出的内存数据会出现错误（BUG!!!!!!!!!!!!!）
#if 1
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
#endif

	move		t5, s5
1:
	beq			t5, s6, 2f
	nop

	ld			t3, 0x00(t5)
	ld			t4, 0x00(t6)
	beq			t3, t4, 11f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x00
	daddu		t5, -0x00

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

11:
	ld			t3, 0x08(t5)
	ld			t4, 0x08(t6)
	beq			t3, t4, 12f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x08
	daddu		t5, -0x08

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

12:
	ld			t3, 0x10(t5)
	ld			t4, 0x10(t6)
	beq			t3, t4, 13f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x10
	daddu		t5, -0x10

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

13:
	ld			t3, 0x18(t5)
	ld			t4, 0x18(t6)
	beq			t3, t4, 14f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x18
	daddu		t5, -0x18

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

14:
	ld			t3, 0x20(t5)
	ld			t4, 0x20(t6)
	beq			t3, t4, 15f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x20
	daddu		t5, -0x20

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

15:
	ld			t3, 0x28(t5)
	ld			t4, 0x28(t6)
	beq			t3, t4, 16f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x28
	daddu		t5, -0x28

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

16:
	ld			t3, 0x30(t5)
	ld			t4, 0x30(t6)
	beq			t3, t4, 17f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x30
	daddu		t5, -0x30

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

17:
	ld			t3, 0x38(t5)
	ld			t4, 0x38(t6)
	beq			t3, t4, 18f
	nop
	bal			do_error_mark						/* 读出到有不匹配的数据 */
	daddiu		t5, t5, 0x38
	daddu		t5, -0x38

#ifdef DEBUG_MEM_TEST
	beqz		t7, next_pattern
	daddiu		t7, t7, -1
#endif

18:
	b			1b
	daddiu		t5,	TM_MEM_INTERVAL 

2:
	b			next_pattern
	daddiu		t6, 0x40		

no_pattern:
	b			mem_test_begin						/* 对下一个rank进行测试操作 */
	nop	

mem_test_end:

#ifdef DEBUG_MEM_TEST
	MT_DEBUG("memory test over!\r\n")
	MT_DEBUG("Test Result: v0 = 0x")
	dsrl		a0, s2, 32
	bal			serial_puthex
	nop
	move		a0, s2
	bal			serial_puthex
	nop
	MT_DEBUG(", v1 = 0x")
	dsrl		a0, s3, 32
	bal			serial_puthex
	nop
	move		a0, s3
	bal			serial_puthex
	nop
	MT_DEBUG("\r\n")
#endif	

	move		v0, s2
	move		v1, s3

	move		ra, t9

	/* 恢复局部变量 */	
	GET_TM_STACK_BASE_TO_a0
	ld			t0, 0x00(a0)
	ld			t1, 0x08(a0)
	ld			t2, 0x10(a0)
	ld			t3, 0x18(a0)
	ld			t4, 0x20(a0)
	ld			t5, 0x28(a0)
	ld			t6, 0x30(a0)
	ld			t7, 0x38(a0)
	ld			t8, 0x40(a0)
	ld			t9, 0x48(a0)
	ld			s0, 0x50(a0)
	ld			s1, 0x58(a0)
	ld			s2, 0x60(a0)
	ld			s3, 0x68(a0)
	ld			s4, 0x70(a0)
	ld			s5, 0x78(a0)
	ld			s6, 0x80(a0)
	ld			s7, 0x88(a0)

	jr			ra
	nop
	.end	mem_test

/*
 * 描述：内存测试读出数据出错处理
 * 参数：t3:实际内存读出数据 t4:写入内存的原始数据 t5:当前数据所在地址
 * 寄存器使用：s2:标记出错的位 s3:读出错误时，标记重读与第一次读出错的位
 * 返回值：
 */
	.global	do_error_mark
	.ent	do_error_mark
do_error_mark:
	move		t8 ,ra
	
	xor			a0, t3, t4
	or			s2, a0

	cache		Hit_Invalidate_D, 0x00(t5)
	cache		Hit_Invalidate_S, 0x00(t5)
	sync		
	
	ld			t0, 0x00(t5)	

#ifdef DEBUG_MEM_TEST
	MT_DEBUG("Addr: 0x")
	dsrl		a0, t5, 32
	bal			serial_puthex
	nop
	move		a0, t5
	bal			serial_puthex
	nop
	MT_DEBUG(", Expected: 0x")
	dsrl		a0, t4, 32
	bal			serial_puthex
	nop
	move		a0, t4
	bal			serial_puthex
	nop
	MT_DEBUG(", Read: 0x")
	dsrl		a0, t3, 32
	bal			serial_puthex
	nop
	move		a0, t3
	bal			serial_puthex
	nop
	MT_DEBUG(", Re-Read: 0x")
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
#endif
	
	xor			a0, t0, t3
	beqz		a0, 1f
	or			s3, s3, a0
#ifdef DEBUG_MEM_TEST
	MT_DEBUG(" *")
#endif

1:
#ifdef DEBUG_MEM_TEST
	MT_DEBUG("\r\n")
#endif

	jr			t8
	nop
	.end	do_error_mark

	
