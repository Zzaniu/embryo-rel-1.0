
/**
 * Copyright(c) 2015-7-16 Shangwen Wu	
 *
 * LS2H DDR控制器参数校准（仅考虑MC0） 
 * 
 */

#include "ddr_auto_level.h"

//#define DEBUG_AUTO_LEVEL							//调试开关
#ifdef DEBUG_AUTO_LEVEL
#define AL_DEBUG(str)						\
	.rdata; 101: .asciz str; .text; la a0, 101b; bal serial_puts; nop
#else
#define AL_DEBUG(str)	;	
#endif


/*
 * 描述：内存读写校准
 * 参数：t7表示DDR当前节点配置寄存器首地址
 * 返回：
 * 寄存器使用：t8（返回地址）t7（MC配置基址）t6（校准数据地址）
 */
	.global	ddr_auto_level
	.ent	ddr_auto_level
ddr_auto_level:
	move		t8, ra
	
	AL_DEBUG("\r\nstart auto SDRAM leveling...\r\n")	

	/* 设置校准使用的地址窗口，内存地址被映射到0x1000000000~0x1fffffffff */
	sync
	nop
	nop
	nop
	nop
	GET_LEVEL_XBAR_REG_TO_a0
	GET_LEVEL_XBAR_BASE_TO_a1
	sd			a1, XBAR_WIN_BASE_OFFSET(a0)	
	dli			a1, LEVEL_XBAR_MASK
	sd			a1, XBAR_WIN_MASK_OFFSET(a0)	
	dli			a1, LEVEL_XBAR_MMAP
	sd			a1, XBAR_WIN_MMAP_OFFSET(a0)	
	sync
	nop
	nop
	nop
	nop

	/* lock 0x0~0x0地址范围内的二级缓存 */		
	GET_LEVEL_SC_LOCK_REG_TO_a0
	dli			a1, LEVEL_SC_LOCK_MASK
	sd			a1, SC_LOCK_MASK_REG(a0)		
	GET_LEVEL_SC_LOCK_BASE_TO_a1
	sd			a1, SC_LOCK_BASE_REG(a0)

	/* 保存局部变量 */
	GET_LEVEL_STACK_BASE_TO_a0
	sd			t0, 0x00(a0)
	sd			t1, 0x08(a0)
	sd			t2, 0x10(a0)
	sd			t3, 0x18(a0)
	sd			t4, 0x20(a0)
	sd			t5, 0x28(a0)
	sd			t6, 0x30(a0)
	sd			s0, 0x38(a0)						//don't change
	sd			s1, 0x40(a0)						//don't change
	sd			s2, 0x48(a0)
	sd			s3, 0x50(a0)
	sd			s4, 0x58(a0)
	sd			s5, 0x60(a0)
	sd			s6, 0x68(a0)
	sd			s7, 0x70(a0)

	GET_LEVEL_STORE_BASE_TO_a0
	move		t6, a0	

/*
 * 描述：写校准过程
 * 寄存器使用：s5临时保存测试结果，t3,t4,t5函数调用参数，
 *             s2测试值对应的偏移以及索引,s3临时保存t5
 *			   s4判断是否进行pop_delay_alter 
 *			   t6寄存器数据校准地址，t7为MC配置寄存器基址
 */
wrlvl_begin:
	AL_DEBUG("start write leveling, wait a while...\r\n")
	
	//prepare for write leveling
	dli			t3, 0x0f							//for all registers	
	dli			t4, 0x05							//wrlvl_dq_delay = 0x18
	dli			t5, WRLVL_DQ_SMALL_TEMP_DELAY
	bal 		ddr_param_modify
	nop

wrlvl_start:
	//清除store空间
	sd			zero, B0_TM_ERR(t6)	
	sd			zero, B1_TM_ERR(t6)	
	sd			zero, B2_TM_ERR(t6)	
	sd			zero, B3_TM_ERR(t6)	
	sd			zero, B4_TM_ERR(t6)	
	sd			zero, B5_TM_ERR(t6)	
	sd			zero, B6_TM_ERR(t6)	
	sd			zero, B7_TM_ERR(t6)	
	sd			zero, WRLVL_DELAY_MAX(t6)
	sd			zero, WRLVL_DELAY_MIN(t6)
	sd			zero, WRLVL_DELAY_MID(t6)

	//initialize  
	dli			t5, WRLVL_MAX_DELAY_VALUE
	dsrl		s2, t5, LEVELING_STEP
	move		s5, zero							
	not			s5, s5

10:
	//设置wrlvl_delay		
	dli			t4, 0x04
	bal 		ddr_param_modify
	nop

	move		s3, t5								//临时保存t5

	move		s4, zero
11:
	dli			t5, 0x00	
12:
	//设置phy_ctrl_0_gate
	dli			t4, 0x06	
	bal 		ddr_param_modify
	nop
	
	//测试以及当前结果处理
	bal 		mem_test
	nop
	//检测每个字节对应的结果位是否为0，如果无错则清除s5对应字节位置的值
	dli			a0, 0xff00000000000000
1:
	and			a1, v0, a0
	bnez		a1, 2f
	nop
	not			a2, a0
	and			s5, a2	
2:
	dsrl		a0, 8
	bnez		a0, 1b
	nop

	//测试下一个phy_ctrl_0_gate值(测试值0,1,2)
	dli			a3, 0x03
	daddu		t5, 1
	blt			t5, a3, 12b
	nop

	//修改读操作pop_delay_alter参数重复测试
	bnez		s4, 13f
	daddiu		s4, s4, 1	

	bal			ddr_modify_pop_delay_alter
	nop

	b			11b
	nop
	
13:
	move		t5, s3								//恢复t5

	/* 保存当前wrlvl_delay值的测试结果 */	
	dli			a0, 0xff00000000000000
	daddu		t0, t6, B7_TM_ERR
1:
	and			a1, s5, a0
	beqz		a1, 2f
	nop
	dli			a2, 0x01
	dsll		a2, s2
	ld			a3, 0x00(t0)
	or			a3, a2
	sd			a3, 0x00(t0)
2:
	dsrl		a0, 8
	bnez		a0, 1b
	daddiu		t0, t0, -0x08						//注意，写入地址需要加上每个字节对应的偏移

	//测试下一个wrlvl_delay值
	dsubu		t5, 1 << LEVELING_STEP
	bltz		t5, 14f
	nop	

	b			10b
	daddiu		s2, -1	

14:
/*
 * 计算每个字节的中间窗口值 
 * 寄存器使用：s2保存当前第几个字节，s3保存当前字节对应的Bn_TM_ERR地址，
 *			   s4记录未找到窗口值的次数
 */
	move		s4, zero							//窗口值未找到统计
	dli			s2, 7

	MC0_GET_INFO_TO_a1(DIMM_WIDTH)
	beqz		a1, 20f
	nop
	//32bit DIMM
	ld			a0, B0_TM_ERR(t6)
	ld			a2, B4_TM_ERR(t6)
	or			a0, a2
	sd			a0, B0_TM_ERR(t6)	

	ld			a0, B1_TM_ERR(t6)
	ld			a2, B5_TM_ERR(t6)
	or			a0, a2
	sd			a0, B1_TM_ERR(t6)	

	ld			a0, B2_TM_ERR(t6)
	ld			a2, B6_TM_ERR(t6)
	or			a0, a2
	sd			a0, B2_TM_ERR(t6)	

	ld			a0, B3_TM_ERR(t6)
	ld			a2, B7_TM_ERR(t6)
	or			a0, a2
	sd			a0, B3_TM_ERR(t6)	

	dli			s2, 3
20:
	daddu		s3, t6, B0_TM_ERR
	dsll		a0, s2, 3
	daddu		s3, a0
	
	move		t0, zero
	dli			a1, WRLVL_MAX_DELAY_VALUE
	dsrl		t1, a1, LEVELING_STEP
	dli			t2, LEVELING_WINDOW_VALUE

	ld			a0, 0x00(s3)
1:
	dli			a1, 1
	dsll		a1, a1, t0
	and			a2, a0, a1
	bnez		a2, 2f
	nop
	//找到一个测试通过值
	daddu		t2, -1
	beqz		t2, 4f
	nop
	b			3f
	nop

2:
	//找到一个测试出错值，清空窗口值计数
	dli			t2, LEVELING_WINDOW_VALUE	
	
3:
	//搜索下一个测试结果
	blt			t0, t1, 1b
	daddiu		t0, t0, 1

	//搜索完毕，未找到窗口值
	daddu		s4, 1
	
	b			5f
	nop

4:
	//找到一个窗口值
	dsubu		a1, t0, LEVELING_WINDOW_VALUE - 1	//note!!!	
	dsll		a1, LEVELING_STEP
	dsll		a3, s2, 3
	dsll		a1, a3
	ld			a2, WRLVL_DELAY_MIN(t6)
	or			a2, a1
	sd			a2, WRLVL_DELAY_MIN(t6)				/* 写入最小wrlvl_delay */

	//继续向上找到测试通过值上界
1:
	bge			t0, t1, 2f
	daddiu		t0, t0, 1
	
	dli			a2, 1
	dsll		a2, a2, t0
	and			a2, a0, a2
	bnez		a2, 2f
	nop

	b			1b
	nop

2:	
	//到达测试值边界，或找到一个失败值
	dsubu		a1, t0, 1							//note!!!
	dsll		a1, LEVELING_STEP
	dsll		a3, s2, 3
	dsll		a1, a3
	ld			a2, WRLVL_DELAY_MAX(t6)
	or			a2, a1
	sd			a2, WRLVL_DELAY_MAX(t6)				/* 写入最大wrlvl_delay */

5:
	//遍历下一个字节
	bnez		s2,	20b
	daddiu		s2, s2, -1

	/* 测试结果处理完毕，开始计算中间值 */
	beqz		s4, 2f
	nop

	/* 有byte未找到窗口值 */	
	PRINTSTR("write leveling error, try another pad compensation\r\n")	
	bal			ddr_modify_pad_comp
	nop
	bnez		v0, 1f
	nop
	b			wrlvl_start
	nop

1:
	/* 尝试pad补偿出错，写入默认值 */
	PRINTSTR("write leveling failed, write default value!\r\n")	
	dli			a1, WRLVL_DEFAULT_DELAY_VALUE
	sd			a1, WRLVL_DELAY_MID(t6)
	b			wrlvl_mid_caled
	nop

2:
	AL_DEBUG("window found! start calculate mid value for each byte lane\r\n")
	
	ld			a1, WRLVL_DELAY_MIN(t6)
	ld			a2, WRLVL_DELAY_MAX(t6)
	daddu		t0, a1, a2	
	dsrl		t0, 1
	and			t0, 0x8080808080808080

	MC0_GET_INFO_TO_a1(SDRAM_TYPE)	
	dli			a0, 0x03
	beq			a1, a0, 31f
	nop

	//DDR2, 根据byte0的边界确定延迟值
	ld			a1, WRLVL_DELAY_MIN(t6)
	ld			a2, WRLVL_DELAY_MAX(t6)
	dsrl		a0, a1, 0
	and			a0, 0x7f
	beqz		a0, 30f
	nop
	
	dsrl		a3, a2, 0
	and			a3, 0x7f
	dli			a0, (WRLVL_MAX_DELAY_VALUE - 0x04)	
	blt			a3, a0, 1f
	nop
	
	//min > 0 && max >= (0x68 - 0x04)，MIN大于下边界，MAX抵达上边界，使用(mid+max)/2 
	daddu		a0, a2, t0
	dsrl		a0, 1
	and			a0, 0x8080808080808080
	
	daddu		t0, t0, a0
	dsrl		t0, 1
	and			t0, 0x8080808080808080

	b			38f
	nop

1:
	//min > 0 && max < (0x68 - 0x04)，MIN大于下边界，MAX小于上边界，使用mid
	b			38f
	nop

30:
	dsrl		a3, a2, 0
	and			a3, 0x7f
	dli			a0, (WRLVL_MAX_DELAY_VALUE - 0x04)	
	blt			a3, a0, 1f
	nop
	
	//min = 0 && max >= (0x68 -0x04)，MAX、MIN均抵达边界，使用mid
	b			38f
	nop

1:
	//min = 0 && max < (0x68 - 0x04), MIN抵达下边界，MAX小于上边界，使用max-OFFSET
	dsubu		t0, a2, 0x3030303030303030
	and			t0, 0x7f7f7f7f7f7f7f7f

	b			38f
	nop

31:
	//DDR3 
	MC0_GET_INFO_TO_a1(DIMM_TYPE)	
	bnez		a1, 32f			
	nop

	//DDR3 UDIMM, MID = OFFSET + shift(byte0 max - min(byte0 max, minus))
	ld			a0, WRLVL_DELAY_MAX(t6)
	and		 	a0, 0x7f
	move		a1, a0
	dli			a2, WRLVL_MINUS_DELAY_VALUE
	ble			a1, a2, 1f
	nop
	
	move		a1, a2
1:	
	dsubu		a0,	a1
	/* 将上述计算的差值移位到每个字节 */
	dsll		a3, a0, 8
	or			a0, a3
	dsll		a3, a0, 16
	or			a0, a3
	dsll		a3, a0, 32
	or			a0, a3

	daddu		t0, a0, WRLVL_DDR3_UDIMM_DEFAULT_OFFSET

	b			38f
	nop

32:
	//DDR3 RDIMM, MID = max - shift(min(byte3 max, byte4 max, byte5 max, minus))
	ld			a0, WRLVL_DELAY_MAX(t6)
	dsrl		a1, a0, 32
	dsrl		a2, a0, 24
	and			a1, 0x7f
	and			a2, 0x7f
	ble			a1, a2, 1f						//比较byte3, byte4	
	nop
	
	move		a1, a2
1:
	dsrl		a2, a0, 40
	and			a2, 0x7f
	ble			a1, a2, 1f						//比较a1, byte5
	nop
	
	move		a1, a2
1:
	dli			a2, WRLVL_MINUS_DELAY_VALUE
	ble			a1, a2, 1f
	nop

	move		a1, a2
1:
	dsll		a3, a1, 8
	or			a1, a3
	dsll		a3, a1, 16
	or			a1, a3
	dsll		a3, a1, 32
	or			a1, a3

	dsub		a2, a0, a1
	and			a3, a2, 0x7f7f7f7f7f7f7f7f
	bnez		a3, 1f
	nop
	
	move		t0, a2

	b			38f
	nop

1:
	//abnormal, 某些byte小于min(byte3 max, byte4 max, byte5 max, minus)
	dsrl		t0, 1
	and			t0, 0x8080808080808080

	b			38f
	nop

38:
	sd			t0, WRLVL_DELAY_MID(t6)

wrlvl_mid_caled:	

#ifdef DEBUG_AUTO_LEVEL
	AL_DEBUG("\r\nwrite leveling info:\r\n")
	AL_DEBUG("wrlvl_delay max = 0x")
	ld			t0, WRLVL_DELAY_MAX(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nwrlvl_delay min = 0x")
	ld			t0, WRLVL_DELAY_MIN(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nwrlvl_delay mid = 0x")
	ld			t0, WRLVL_DELAY_MID(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\n")
#endif

wrlvl_end:
	ld			t3, WRLVL_DELAY_MID(t6)
	bal			ddr_write_wrlvl_delay
	nop
	
	dli			t3, 0x0f
	dli			t4, 0x05
	dli			t5, WRLVL_DQ_DEFAULT_DELAY
	bal			ddr_param_modify
	nop

	AL_DEBUG("Write Leveling complete!\r\n")

#ifdef DEBUG_AUTO_LEVEL
	AL_DEBUG("\r\nAfter write leveling, the MC0 param is:\r\n")	
	bal			enable_ddr_config_space
	nop

    dli			t0, LS2H_DDR_PARAM_NUM 
    move	    t1, t7
1:
	AL_DEBUG("Offset 0x")
	move		a0, t1
	bal			serial_puthex
	nop
	AL_DEBUG(", Value ")	
    ld      	t2, 0x0(t1)
    dsrl   	 	a0, t2, 32
    bal     	serial_puthex
    nop
    AL_DEBUG("  ")
    move   		a0, t2
    bal     	serial_puthex
    nop
    AL_DEBUG("\r\n")

    daddu  		t0, -1
    bnez    	t0, 1b
    daddiu  	t1, t1, 0x10
	AL_DEBUG("\r\n")

	bal			disable_ddr_config_space
	nop
#endif

/*
 * 描述：读校准过程采用单字节操作，这点与写校准不同
 * 寄存器使用：s3保存当前phy1_gate值，t3,t4,t5函数调用参数，
 *             s2测试值对应的偏移以及索引
 *			   s4判断是否进行pop_delay_alter，s5当前字节掩码 
 *			   t6寄存器数据校准地址，t7为MC配置寄存器基址
 */
rdlvl_begin:
	AL_DEBUG("start read leveling, wait a while...\r\n")

	dli			t3, 7
	MC0_GET_INFO_TO_a1(DIMM_WIDTH)
	beqz		a1, 1f 
	nop
	dli			t3, 3
1:
	dli			s5, 0xff
	dsll		a0, t3, 3
	dsll		s5, a0
	MC0_GET_INFO_TO_a1(DIMM_WIDTH)
	beqz		a1, rdlvl_one_byte_start 
	nop

	dsll		a2, s5, 32
	or			s5, a2

rdlvl_one_byte_start:
	sd			zero, BYTE_TM_ERR(t6)
	sd			zero, RDLVL_PHY_1_GATE_DELAY(t6)
	sd			zero, RDLVL_GATE_DELAY_MAX(t6)
	sd			zero, RDLVL_GATE_DELAY_MIN(t6)
	sd			zero, RDLVL_GATE_DELAY_MID(t6)
	sd			zero, RDLVL_DQS_P_DELAY_MAX(t6)
	sd			zero, RDLVL_DQS_P_DELAY_MIN(t6)
	sd			zero, RDLVL_DQS_N_DELAY_MAX(t6)
	sd			zero, RDLVL_DQS_N_DELAY_MIN(t6)
	sd			zero, RDLVL_FAIL_MARK(t6)	
	
	move		s4, zero
	
rdlvl_pop_delay_start:
	/* 写入rdlvl_dqsp/n_delay默认值0x20 */
	dli			t5, 0x20
	dli			t4, 2
	bal			ddr_param_modify
	nop
	daddu		t4, 1
	bal			ddr_param_modify
	nop

	dli			s3, 2

rdlvl_phy1_gate_delay_start:
	dli			t4, 6
	move		t5, s3
	bal			ddr_param_modify
	nop
	
	dli			a0, RDLVL_GATE_MAX_DELAY_VALUE
	dsrl		s2, a0, LEVELING_STEP	
	sd			zero, BYTE_TM_ERR(t6)
			
rdlvl_gate_delay_start:
	dsll		t5, s2, LEVELING_STEP
	dli			t4, 1
	bal			ddr_param_modify
	nop
	
	bal			mem_test
	nop

	move		a0, v0	
	and			a0, s5
	beqz		a0, 1f
	nop
	ld			a1, BYTE_TM_ERR(t6)					//error detected
	dli			a2, 0x01
	dsll		a2, s2
	or			a1, a2
	sd			a1, BYTE_TM_ERR(t6)
	
1:
	bgtz		s2, rdlvl_gate_delay_start
	daddiu		s2, s2, -1
	
//rdlvl_gate_delay_end:
/*
 * 计算当前字节的rdlvl_gate中间延迟值
 * 寄存器使用：
 */
	move		t0, zero
	dli			a1, RDLVL_GATE_MAX_DELAY_VALUE
	dsrl		t1, a1, LEVELING_STEP 
	dli			t2, LEVELING_WINDOW_VALUE + RDLVL_WINDOW_ADJUST

	ld			a0, BYTE_TM_ERR(t6)
1:
	dli			a1, 1
	dsll		a1, a1, t0
	and			a2, a0, a1
	bnez		a2, 2f
	nop
	//找到一个测试通过值
	daddu		t2, -1
	beqz		t2, 4f
	nop
	b			3f
	nop

2:
	//找到一个测试出错值，清空窗口值计数
	dli			t2, LEVELING_WINDOW_VALUE + RDLVL_WINDOW_ADJUST	
	
3:
	//搜索下一个测试结果
	blt			t0, t1, 1b
	daddiu		t0, t0, 1

	//搜索完毕，未找到窗口值，尝试下一个phy1_delay
	bnez		s3, rdlvl_phy1_gate_delay_start
	daddiu		s3, s3, -1
	
	/* 所有phy1_delay均已测试，还没有找到rdlvl_gat_delaye和窗口值，置失败标记 */
	ld			a1, RDLVL_FAIL_MARK(t6)
	dli			a2, 0x01
	dsll		a2, s4
	or			a1, a2
	sd			a1, RDLVL_FAIL_MARK(t6)

	b			rdlvl_pop_delay_end
	nop

4:
	//找到一个窗口值，先保存当前可行的phy1_gate_delay
	ld			a1, RDLVL_PHY_1_GATE_DELAY(t6)
	dsll		a2, s3, s4
	or			a1, a2
	sd			a1, RDLVL_PHY_1_GATE_DELAY(t6)

	dsubu		a1, t0, LEVELING_WINDOW_VALUE + RDLVL_WINDOW_ADJUST - 1	//note!!!	
	dsll		a1, LEVELING_STEP
	dsll		a3, s4, 3							/* 注意MIN值要写到对应pop_delay位置 */
	dsll		a1, a3
	ld			a2, RDLVL_GATE_DELAY_MIN(t6)
	or			a2, a1
	sd			a2, RDLVL_GATE_DELAY_MIN(t6)		/* 写入最小rdlvl_gate_delay */

	//继续向上找到测试通过值上界
1:
	bge			t0, t1, 2f
	daddiu		t0, t0, 1
	
	dli			a2, 1
	dsll		a2, a2, t0
	and			a2, a0, a2
	bnez		a2, 2f
	nop

	b			1b
	nop

2:	
	//到达测试值边界，或找到一个失败值
	dsubu		a1, t0, 1							//note!!!
	dsll		a1, LEVELING_STEP
	dsll		a3, s4, 3							/* 写入pop_delay对应位置 */
	dsll		a1, a3
	ld			a2, RDLVL_GATE_DELAY_MAX(t6)
	or			a2, a1
	sd			a2, RDLVL_GATE_DELAY_MAX(t6)		/* 写入最大rdlvl_gate_delay */

	/* 计算中间值，若max == MAX_DELAY_VALUE，mid = (mid + max) / 2，否则mid = max - 10 */
	ld			a1, RDLVL_GATE_DELAY_MIN(t6) 
	ld			a2, RDLVL_GATE_DELAY_MAX(t6)
	dsll		a3, s4, 3
	dsrl		a1, a3
	dsrl		a2, a3
	and			a1, 0xff
	and			a2, 0xff

	dli			a3, RDLVL_GATE_MAX_DELAY_VALUE
	beq			a2, a3, 1f
	nop
	
	dli			a3, 0x10
	dsubu		a3, a2, a3
	bge			a3, zero, 2f
	nop
	move		a3, zero
	b			2f
	nop		

1:
	daddu		a3, a1, a2
	dsrl		a3, 1
	daddu		a3, a2
	dsrl		a3, 1

2:
	/* 计算完成保存中间值，并写入MC配置寄存器 */
	move		t5, a3
	dsll		a1, s4, 3							/* 写入pop_delay对应位置 */
	dsll		a3, a1
	ld			a2, RDLVL_GATE_DELAY_MID(t6)
	or			a2, a3
	sd			a2, RDLVL_GATE_DELAY_MID(t6)		/* 写入rdlvl_gate_delay */

	bal			ddr_param_modify
	nop		
	
/*
 * 下面开始进行rdlvl_dqsP/N_delay校准
 * 寄存器使用：s2测试值对应的偏移以及索引
 *
 */
	dli			t4, 0x02

40:
	dli			a0, RDLVL_MAX_DELAY_VALUE
	dsrl		s2, a0, LEVELING_STEP	
	sd			zero, BYTE_TM_ERR(t6)

41:
	dsll		t5, s2, LEVELING_STEP
	bal			ddr_param_modify
	nop

	bal			mem_test
	nop

	move		a0, v0	
	and			a0, s5
	beqz		a0, 1f
	nop
	ld			a1, BYTE_TM_ERR(t6)					//error detected
	dli			a2, 0x01
	dsll		a2, s2
	or			a1, a2
	sd			a1, BYTE_TM_ERR(t6)
	
1:
	bgtz		s2, 41b 
	daddiu		s2, s2, -1
	
/*
 * 计算当前字节的rdlvl_dqsP/N中间延迟值
 * 寄存器使用：
 */
	move		t0, zero
	dli			a1, RDLVL_MAX_DELAY_VALUE
	dsrl		t1, a1, LEVELING_STEP 
	dli			t2, LEVELING_WINDOW_VALUE 

	ld			a0, BYTE_TM_ERR(t6)
1:
	dli			a1, 1
	dsll		a1, a1, t0
	and			a2, a0, a1
	bnez		a2, 2f
	nop
	//找到一个测试通过值
	daddu		t2, -1
	beqz		t2, 4f
	nop
	b			3f
	nop

2:
	//找到一个测试出错值，清空窗口值计数
	dli			t2, LEVELING_WINDOW_VALUE 
	
3:
	//搜索下一个测试结果
	blt			t0, t1, 1b
	daddiu		t0, t0, 1
	
	/* 搜索结束，未找到窗口值，将置标识位，并退出 */
	ld			a1, RDLVL_FAIL_MARK(t6)
	dli			a2, 0x01
	dsll		a2, s4
	or			a1, a2
	sd			a1, RDLVL_FAIL_MARK(t6)

	b			rdlvl_pop_delay_end
	nop

4:
	//找到一个窗口值
	dsubu		a1, t0, LEVELING_WINDOW_VALUE - 1	//note!!!	
	dsll		a1, LEVELING_STEP
	dsll		a3, s4, 3							/* 注意MIN值要写到对应pop_delay位置 */
	dsll		a1, a3

	dli			a3, 0x03
	beq			t4, a3, 1f 
	nop
	ld			a2, RDLVL_DQS_P_DELAY_MIN(t6)
	or			a2, a1
	sd			a2, RDLVL_DQS_P_DELAY_MIN(t6)		/* 写入最小rdlvl_dqsp_delay */
	b			2f
	nop
1:
	ld			a2, RDLVL_DQS_N_DELAY_MIN(t6)
	or			a2, a1
	sd			a2, RDLVL_DQS_N_DELAY_MIN(t6)		/* 写入最小rdlvl_dqsn_delay */

	//继续向上找到测试通过值上界
2:	
	bge			t0, t1, 2f
	daddiu		t0, t0, 1
	
	dli			a2, 1
	dsll		a2, a2, t0
	and			a2, a0, a2
	bnez		a2, 3f
	nop

	b			2b
	nop

3:	
	//到达测试值边界，或找到一个失败值
	dsubu		a1, t0, 1							//note!!!
	dsll		a1, LEVELING_STEP
	dsll		a3, s4, 3							/* 写入pop_delay对应位置 */
	dsll		a1, a3

	dli			a3, 0x03
	beq			t4, a3, 1f 
	nop
	ld			a2, RDLVL_DQS_P_DELAY_MAX(t6)
	or			a2, a1
	sd			a2, RDLVL_DQS_P_DELAY_MAX(t6)		/* 写入最大rdlvl_dqsp_delay */
	b			2f
	nop
1:
	ld			a2, RDLVL_DQS_N_DELAY_MAX(t6)
	or			a2, a1
	sd			a2, RDLVL_DQS_N_DELAY_MAX(t6)		/* 写入最大rdlvl_dqsn_delay */

2:	
	/* 恢复当前参数值为0x20 */
	dli			t5, 0x20
	bal			ddr_param_modify
	nop

	dli			a0, 0x03
	blt			t4, a0, 40b
	daddiu		t4, t4, 1

rdlvl_pop_delay_end:
	bnez		s4, 1f 
	nop
	bal			ddr_modify_pop_delay_alter
	nop
	b			rdlvl_pop_delay_start
	daddiu		s4, s4, 1	

1:
#ifdef DEBUG_AUTO_LEVEL
	AL_DEBUG("\r\nslice ")
	move		a0, t3
	bal			serial_puthex
	nop
	AL_DEBUG(" read leveling info:\r\n")
	AL_DEBUG("rdlvl_phy1_gate_delay = 0x")
	ld			t0, RDLVL_PHY_1_GATE_DELAY(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_gate_delay max = 0x")
	ld			t0, RDLVL_GATE_DELAY_MAX(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_gate_delay min = 0x")
	ld			t0, RDLVL_GATE_DELAY_MIN(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_gate_delay mid = 0x")
	ld			t0, RDLVL_GATE_DELAY_MID(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_dqs_p_delay max = 0x")
	ld			t0, RDLVL_DQS_P_DELAY_MAX(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_dqs_p_delay min = 0x")
	ld			t0, RDLVL_DQS_P_DELAY_MIN(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_dqs_n_delay max = 0x")
	ld			t0, RDLVL_DQS_N_DELAY_MAX(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_dqs_n_delay min = 0x")
	ld			t0, RDLVL_DQS_N_DELAY_MIN(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\nrdlvl_fail_mark = 0x")
	ld			t0, RDLVL_FAIL_MARK(t6)
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	AL_DEBUG("\r\n")
#endif //DEBUG_AUTO_LEVEL

	ld			a0, RDLVL_FAIL_MARK(t6)
	move		a1, zero
	beq			a0, a1, both_pop_delay_ok
	daddiu		a1, a1, 1
	beq			a0, a1, first_pop_delay_ok
	daddiu		a1, a1, 1
	beq			a0, a1, second_pop_delay_ok
	daddiu		a1, a1, 1
	beq			a0, a1, both_pop_delay_fail
	nop

both_pop_delay_ok:
	/* 判断哪一个pop_delay更好 */
	ld			a0, RDLVL_DQS_P_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_P_DELAY_MAX(t6)
	dsubu		a2, a1, a0
	dsrl		s2, a2, 0
	and			s2, 0xff
	dsrl		s3, a2, 8
	and			s3, 0xff	
	ld			a0, RDLVL_DQS_N_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_N_DELAY_MAX(t6)
	dsubu		a2, a1, a0
	dsrl		s4, a2, 0
	and			s4, 0xff
	dsrl		s6, a2, 8
	and			s6, 0xff	

	/* 判断依据是，选择dqsp的max\min差值和dqsp的max\min差值中均较大的那一次pop_delay */
	ble			s2, s4, 1f
	nop
	move		s2, s4
1:
	ble			s3, s6, 1f
	nop
	move		s3, s6
1:
#if 1
	//仅对于DDR3 DIMM，给予第二次pop_delay更大优先级
	dsubu		s2, 0x08
#endif

	ble			s2, s3, second_pop_delay_ok
	nop
		
first_pop_delay_ok:
	AL_DEBUG("slice level success, use first pop value\r\n")	

	bal			ddr_modify_pop_delay_alter			//恢复第一次phy0_gate（pop_delay）值
	nop

	//写入rdlvl_phy1_gate_delay参数	
	dli			t4, 6
	ld			t5, RDLVL_PHY_1_GATE_DELAY(t6)
	dsrl		t5, 0
	and			t5, 0xff
	bal			ddr_param_modify
	nop
	//写入rdlvl_gate_delay参数
	dli			t4, 1
	ld			t5, RDLVL_GATE_DELAY_MID(t6)
	dsrl		t5, 0
	and			t5, 0xff
	bal			ddr_param_modify
	nop
	//写入rdlvl_dqs_p_delay参数
	dli			t4, 2
	ld			a0, RDLVL_DQS_P_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_P_DELAY_MAX(t6)
	dsrl		a0, 0
	dsrl		a1, 0
	and			a0, 0xff
	and			a1, 0xff
	daddu		t5, a0, a1
	dsrl		t5, 1
	bal			ddr_param_modify
	nop
	//写入rdlvl_dqs_n_delay参数
	daddu		t4, 1
	ld			a0, RDLVL_DQS_N_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_N_DELAY_MAX(t6)
	dsrl		a0, 0
	dsrl		a1, 0
	and			a0, 0xff
	and			a1, 0xff
	daddu		t5, a0, a1
	dsrl		t5, 1
	bal			ddr_param_modify
	nop

	b			rdlvl_one_byte_end
	nop

second_pop_delay_ok:	
	AL_DEBUG("slice level success, use second pop value\r\n")	

	//写入rdlvl_dqs_p_delay参数
	dli			t4, 2
	ld			a0, RDLVL_DQS_P_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_P_DELAY_MAX(t6)
	dsrl		a0, 8
	dsrl		a1, 8
	and			a0, 0xff
	and			a1, 0xff
	daddu		t5, a0, a1
	dsrl		t5, 1
	bal			ddr_param_modify
	nop
	//写入rdlvl_dqs_n_delay参数
	daddu		t4, 1
	ld			a0, RDLVL_DQS_N_DELAY_MIN(t6)
	ld			a1, RDLVL_DQS_N_DELAY_MAX(t6)
	dsrl		a0, 8
	dsrl		a1, 8
	and			a0, 0xff
	and			a1, 0xff
	daddu		t5, a0, a1
	dsrl		t5, 1
	bal			ddr_param_modify
	nop

	b			rdlvl_one_byte_end
	nop

both_pop_delay_fail:
	PRINTSTR("slice leveling error, try another pad compensation\r\n")	
	bal			ddr_modify_pad_comp
	nop
	bnez		v0, 1f
	nop
	b			rdlvl_one_byte_start
	nop

1:
	/* 尝试pad补偿出错，写入默认值 */
	PRINTSTR("slice leveling failed, write default value!\r\n")	

	//写入rdlvl_phy1_gate_delay参数	
	dli			t4, 6
	dli			t5, 0x00
	bal			ddr_param_modify
	nop
	//写入rdlvl_gate_delay参数
	dli			t4, 1
	dli			t5, 0x14
	bal			ddr_param_modify
	nop
	//写入rdlvl_dqs_p_delay参数
	dli			t5, 0x20
	dli			t4, 2
	bal			ddr_param_modify
	nop
	//写入rdlvl_dqs_n_delay参数
	daddu		t4, 1
	bal			ddr_param_modify
	nop

rdlvl_one_byte_end:
	dsrl		s5, 8
	blez		t3, rdlvl_end
	daddiu		t3, t3, -1

	b			rdlvl_one_byte_start
	nop

rdlvl_end:
	AL_DEBUG("Read Leveling complete!\r\n")
	
	bal			ddr_write_slice_8_param
	nop

#ifdef DEBUG_AUTO_LEVEL
	AL_DEBUG("\r\nAfter read leveling, the MC0 param is:\r\n")	
	bal			enable_ddr_config_space
	nop

    dli			t0, LS2H_DDR_PARAM_NUM 
    move	    t1, t7
1:
	AL_DEBUG("Offset 0x")
	move		a0, t1
	bal			serial_puthex
	nop
	AL_DEBUG(", Value ")	
    ld      	t2, 0x0(t1)
    dsrl   	 	a0, t2, 32
    bal     	serial_puthex
    nop
    AL_DEBUG("  ")
    move   		a0, t2
    bal     	serial_puthex
    nop
    AL_DEBUG("\r\n")

    daddu  		t0, -1
    bnez    	t0, 1b
    daddiu  	t1, t1, 0x10
	AL_DEBUG("\r\n")

	bal			disable_ddr_config_space
	nop
#endif //DEBUG_AUTO_LEVEL

ddr_auto_level_end:
	/* 确认校准成功 */	
	bal			mem_test
	nop

	beqz		v0, 1f
	nop

	move		t0, v0
	move		t1, v1
	PRINTSTR("error! auto leveling failed!\r\n")
	PRINTSTR("Test result: v0 = 0x")
	dsrl		a0, t0, 32
	bal			serial_puthex
	nop
	move		a0, t0
	bal			serial_puthex
	nop
	PRINTSTR(", v1 = 0x")
	dsrl		a0, t1, 32
	bal			serial_puthex
	nop
	move		a0, t1
	bal			serial_puthex
	nop
	PRINTSTR("\r\n")
		
	dli			v0, 1		
	b			ddr_auto_level_exit
	nop
	
1:
	AL_DEBUG("auto level finished\r\n")
	move		v0, zero

ddr_auto_level_exit:
	/* 恢复局部变量 */	
	GET_LEVEL_STACK_BASE_TO_a0
	ld			t0, 0x00(a0)
	ld			t1, 0x08(a0)
	ld			t2, 0x10(a0)
	ld			t3, 0x18(a0)
	ld			t4, 0x20(a0)
	ld			t5, 0x28(a0)
	ld			t6, 0x30(a0)
	ld			s0, 0x38(a0)
	ld			s1, 0x40(a0)
	ld			s2, 0x48(a0)
	ld			s3, 0x50(a0)
	ld			s4, 0x58(a0)
	ld			s5, 0x60(a0)
	ld			s6, 0x68(a0)
	ld			s7, 0x70(a0)

	/* unlock 0x0~0x0地址范围内的二级缓存 */
	GET_LEVEL_SC_LOCK_REG_TO_a0
	sd			zero, SC_LOCK_MASK_REG(a0)		
	sd			zero, SC_LOCK_BASE_REG(a0)

	/* 清除窗口 */
	sync
	nop
	nop
	nop
	nop
	GET_LEVEL_XBAR_REG_TO_a0
	sd			zero, XBAR_WIN_BASE_OFFSET(a0)	
	sd			zero, XBAR_WIN_MASK_OFFSET(a0)	
	sd			zero, XBAR_WIN_MMAP_OFFSET(a0)	
	sync
	nop
	nop
	nop
	nop

	jr			t8
	nop
	.end	ddr_auto_level

/*************************** 包含其他依赖汇编文件 ***************************/

#include "mem_test.S"


