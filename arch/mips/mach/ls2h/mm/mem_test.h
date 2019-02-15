
/**
 * Copyright(c) 2015-7-21 Shangwen Wu	
 *
 * LS2H DDR内存测试头文件（仅考虑MC0） 
 * 
 */

#ifndef	__DDR_TEST_MEM_H__
#define __DDR_TEST_MEM_H__

/* 
 * 注意，该地址0x1000000400~0x1000000600位于二级缓存中的锁存地址中
 * 0x0~0x200用于auto_level函数栈空间，0x200~0x400用于校准数据保存
 */
#define TM_STACK_BASE		0x9800001000000400		/* 0x400~0x600，64 register */
#define GET_TM_STACK_BASE_TO_a0						\
		dli			a0, CHIP_NODE_ID_MASK;			\
		and			a0, s1;							\
		dsll		a0, 44;							\
		or			a0, TM_STACK_BASE

/* 该地址0x1000000600~0x1000000800位于二级缓存中的锁存地址中,测试数据保存地址 */
#define TM_PATTERN_BASE		0x9800001000000600		/* 0x600~0x1000*/
#define GET_TM_PATTERN_BASE_TO_t6					\
		dli			t6, CHIP_NODE_ID_MASK;			\
		and			t6, s1;							\
		dsll		t6, 44;							\
		or			t6, TM_PATTERN_BASE

/* 内存测试范围设置，下面的配置针对最大内存地址向下大小为0x40000的范围进行间隔为0x400的测试 */
/* 获取内存基地址 */
#define TM_MEM_BASE			0x9800001000000000
#define GET_TM_MEM_BASE_TO_s6						\
		dli			s6, CHIP_NODE_ID_MASK;			\
		and			s6, s1;							\
		dsll		s6, 44;							\
		or			s6, TM_MEM_BASE

/* 测试内存大小 */
#define TM_MEM_LIMIT		0x40000

/* 测试内存间隔 */
#define TM_MEM_INTERVAL		0x400

/* 错误出现最大个数，仅调试用 */
#define TM_MAX_ERRORS		0x10

/*
 * 测试用数据
 * 共八组数据种类，每组8个不同的数据
 * Copyright @loongson
 *
 */

#define TEST_PATTERN_SIZE	8

//1组，“行走的1”
#define PATTERN_D8_0_0  0x0101010101010101
#define PATTERN_D8_0_1  0x0202020202020202
#define PATTERN_D8_0_2  0x0404040404040404
#define PATTERN_D8_0_3  0x0808080808080808
#define PATTERN_D8_0_4  0x1010101010101010
#define PATTERN_D8_0_5  0x2020202020202020
#define PATTERN_D8_0_6  0x4040404040404040
#define PATTERN_D8_0_7  0x8080808080808080
//2组，“行走+反转的1”
#define PATTERN_D8_1_0  0x0101010101010101
#define PATTERN_D8_1_1  0xfefefefefefefefe
#define PATTERN_D8_1_2  0x0202020202020202
#define PATTERN_D8_1_3  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_1_4  0x0404040404040404
#define PATTERN_D8_1_5  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_1_6  0x0808080808080808
#define PATTERN_D8_1_7  0xf7f7f7f7f7f7f7f7
//3组，“行走的0”
#define PATTERN_D8_2_0  0xfefefefefefefefe
#define PATTERN_D8_2_1  0xfdfdfdfdfdfdfdfd
#define PATTERN_D8_2_2  0xfbfbfbfbfbfbfbfb
#define PATTERN_D8_2_3  0xf7f7f7f7f7f7f7f7
#define PATTERN_D8_2_4  0xefefefefefefefef
#define PATTERN_D8_2_5  0xdfdfdfdfdfdfdfdf
#define PATTERN_D8_2_6  0xbfbfbfbfbfbfbfbf
#define PATTERN_D8_2_7  0x7f7f7f7f7f7f7f7f
//4组，“两个行走的0”
#define PATTERN_D8_3_0  0x0000000100000001
#define PATTERN_D8_3_1  0x0000010000000100
#define PATTERN_D8_3_2  0x0001000000010000
#define PATTERN_D8_3_3  0x0100000001000000
#define PATTERN_D8_3_4  0x0100000001000000
#define PATTERN_D8_3_5  0x0001000000010000
#define PATTERN_D8_3_6  0x0000010000000100
#define PATTERN_D8_3_7  0x0000000100000001
//5组，“反转”
#define PATTERN_D8_4_0  0xaaaaaaaaaaaaaaaa
#define PATTERN_D8_4_1  0x5555555555555555
#define PATTERN_D8_4_2  0x0000000800000008
#define PATTERN_D8_4_3  0xfffffff7fffffff7
#define PATTERN_D8_4_4  0x5aa5a55a5aa5a55a
#define PATTERN_D8_4_5  0xa55a5aa5a55a5aa5
#define PATTERN_D8_4_6  0xb5b5b5b5b5b5b5b5
#define PATTERN_D8_4_7  0x4a4a4a4a4a4a4a4a
//6组，“随机”
#define PATTERN_D8_5_0  0x0000000000000000
#define PATTERN_D8_5_1  0xffffffffffffffff
#define PATTERN_D8_5_2  0x55555555aaaaaaaa
#define PATTERN_D8_5_3  0x00000000ffffffff
#define PATTERN_D8_5_4  0x1616161616161616
#define PATTERN_D8_5_5  0xb5b5b5b5b5b5b5b5
#define PATTERN_D8_5_6  0x5555555755575555
#define PATTERN_D8_5_7  0x00020002fffdfffd
//7组，“无名”
#define PATTERN_DB_0_0  0xaaaaaaaaaaaaaaaa
#define PATTERN_DB_0_1  0x5555555555555555
#define PATTERN_DB_1_0  0x0000000800000008
#define PATTERN_DB_1_1  0xfffffff7fffffff7
#define PATTERN_DB_2_0  0x5aa5a55a5aa5a55a
#define PATTERN_DB_2_1  0xa55a5aa5a55a5aa5
#define PATTERN_DB_3_0  0xb5b5b5b5b5b5b5b5
#define PATTERN_DB_3_1  0x4a4a4a4a4a4a4a4a
//8组，“杂项”
#define PATTERN_JUSTA   0xaaaaaaaaaaaaaaaa
#define PATTERN_JUST5   0x5555555555555555
#define PATTERN_FiveA   0x55555555aaaaaaaa
#define PATTERN_ZEROONE 0x00000000ffffffff
#define PATTERN_L8b10b  0x1616161616161616
#define PATTERN_S8b10b  0xb5b5b5b5b5b5b5b5
#define PATTERN_Five7   0x5555555755575555
#define PATTERN_Zero2fd 0x00020002fffdfffd

#endif //__DDR_TEST_MEM_H__
