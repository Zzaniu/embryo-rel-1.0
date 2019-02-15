
/**
 * Copyright(c) 2015-7-16 Shangwen Wu	
 *
 * LS2H DDR控制器参数校准头文件（仅考虑MC0） 
 * 
 */

#ifndef DDR_AUTO_LEVEL
#define DDR_AUTO_LEVEL

/* leveling配置 */
//#define WRLVL_HALF_CLK_CLEAR

#define LEVEL_XBAR_REG_BASE		0x900000001fd80038
#define GET_LEVEL_XBAR_REG_TO_a0					\
		dli			a0, CHIP_NODE_ID_MASK;			\
		and			a0, s1;							\
		dsll		a0, 44;							\
		or			a0, LEVEL_XBAR_REG_BASE
/* 将CPU物理地址0x1000000000~0x1fffffffff（64G）映射到DDR的0x0~0xfffffffff地址空间中 */
#define GET_LEVEL_XBAR_BASE_TO_a1					\
		dli			a1, CHIP_NODE_ID_MASK;			\
		and			a1, s1;							\
		dsll		a1, 44;							\
		or			a1, 0x0000001000000000
#define LEVEL_XBAR_MASK			0xfffffff000000000
#define LEVEL_XBAR_MMAP			0x00000000000000f0

#define SC_LOCK_REG_BASE		0x900000001fd84200		/* 二级缓存锁地址 */
#define SC_LOCK_BASE_REG		0x00	
#define SC_LOCK_MASK_REG		0x40
#define GET_LEVEL_SC_LOCK_REG_TO_a0					\
		dli			a0, CHIP_NODE_ID_MASK;			\
		and			a0, s1;							\
		dsll		a0, 44;							\
		or			a0, SC_LOCK_REG_BASE
/* 将CPU物理地址0x1000000000~0x1000001000（4K）锁存到二级缓存 */
#define GET_LEVEL_SC_LOCK_BASE_TO_a1				\
		dli			a1, CHIP_NODE_ID_MASK;			\
		and			a1, s1;							\
		dsll		a1, 44;							\
		or			a1, 0x8000001000000000
#define LEVEL_SC_LOCK_MASK		0xfffffffffffff000

/* 
 * 下面的空间为何能在内存未完成初始化之前就能够使用呢？ 
 * 注意，该地址0x1000000000~0x1000000200刚好位于上面二级缓存中的锁存地址中
 * ，也就是说，实际上使用下面的地址时，实际上访问的是上面的二级cache，
 * 而不是DDR，而cache早已经初始化完毕  
 */
#define LEVEL_STACK_BASE		0x9800001000000000		/* 函数调用时用于保存调用函数的局部变量，0x0~0x200，64 register */
#define GET_LEVEL_STACK_BASE_TO_a0					\
		dli			a0, CHIP_NODE_ID_MASK;			\
		and			a0, s1;							\
		dsll		a0, 44;							\
		or			a0, LEVEL_STACK_BASE

#define LEVEL_STORE_BASE		0x9800001000000200		/* 保存校准数据，0x200~0x400 */
#define GET_LEVEL_STORE_BASE_TO_a0					\
		dli			a0, CHIP_NODE_ID_MASK;			\
		and			a0, s1;							\
		dsll		a0, 44;							\
		or			a0, LEVEL_STORE_BASE

/* store校准数据保存地址偏移 */
#define B0_TM_ERR						(0x00)			/* 每个字节对应的测试检测出错位 */
#define B1_TM_ERR						(0x08)
#define B2_TM_ERR						(0x10)
#define B3_TM_ERR						(0x18)
#define B4_TM_ERR						(0x20)
#define B5_TM_ERR						(0x28)
#define B6_TM_ERR						(0x30)
#define B7_TM_ERR						(0x38)
#define WRLVL_DELAY_MAX					(0x40)			/* 保存校准的wrlvl_delay值 */
#define WRLVL_DELAY_MIN					(0x48)
#define WRLVL_DELAY_MID					(0x50)

/* 注意以下存储空间与写校准的存储空间复用 */
#define BYTE_TM_ERR						(0x00)
#define RDLVL_PHY_1_GATE_DELAY			(0x08)
#define RDLVL_GATE_DELAY_MAX			(0x10)
#define RDLVL_GATE_DELAY_MIN			(0x18)
#define RDLVL_GATE_DELAY_MID			(0x20)
#define RDLVL_DQS_P_DELAY_MAX			(0x28)
#define RDLVL_DQS_P_DELAY_MIN			(0x30)
#define RDLVL_DQS_N_DELAY_MAX			(0x38)
#define RDLVL_DQS_N_DELAY_MIN			(0x40)
#define RDLVL_FAIL_MARK					(0x48)

#define LEVELING_WINDOW_VALUE			8				/* 连续通过测试的最小允许次数 */
#define RDLVL_WINDOW_ADJUST				0			
#define LEVELING_STEP					1				/* 测试延迟值间隔 */

/* Copyright @loongson */
/* 以下常量在更改wrlvl_rq_delay参数时被用到 */
#define WRLVL_DQ_SMALL_TEMP_DELAY		0x18			/* 写校准值时设置 */
#define WRLVL_DQ_DEFAULT_DELAY			0x20			/* 校准完恢复 */

/* 以下常量在更改wrlvl_delay参数时将被用到 */
#define WRLVL_DEFAULT_DELAY_VALUE		0x48423c362a262220
#define WRLVL_MAX_DELAY_VALUE			0x68
#define WRLVL_MINUS_DELAY_VALUE			0x40
#define WRLVL_HALF_CLK_VALUE			0x40
#define DQSDQ_OUT_WINDOW_VALUE			0x3733
#define PHY_DQSDQ_INC_VALUE_80			0x84400
#define PHY_DQSDQ_INC_VALUE_60			0x30400
#define PHY_DQSDQ_INC_VALUE_40			0x30000
#define PHY_DQSDQ_INC_VALUE_00			0x00000
#define WRLVL_DDR3_UDIMM_DEFAULT_OFFSET	0x12120c0c06060000

/* 以下常量在更改rdlvl_gate_delay参数时将被用到 */
#define RDLVL_GATE_MAX_DELAY_VALUE		0x22

/* 以下常量在更改rdlvl_dqsP/N_delay参数时用到 */
#define RDLVL_MAX_DELAY_VALUE			0x50

/* 以下常量在更改phy_ctrl_1_gate参数数将被用到 */
#define CPU_ODT_BASE_VALUE				0x15
#define CPU_ODT_INC_VALUE				0x11





#endif //DDR_AUTO_LEVEL
