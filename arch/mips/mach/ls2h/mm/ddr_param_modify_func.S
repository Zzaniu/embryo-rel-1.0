
/**
 * Copyright(c) 2015-7-10 Shangwen Wu	
 *
 * LS2H 内存控制器配置参数函数集（仅考虑MC0） 
 * 
 */

//#define DEBUG_MODIFY_PARAM					//调试开关
#ifdef DEBUG_MODIFY_PARAM
#define MP_DEBUG(str)						\
	.rdata; 101: .asciz str; .text; la a0, 101b; bal serial_puts; nop
#else
#define MP_DEBUG(str)	;	
#endif

/*
 * 描述：DDR运行修改MC参数配置
 *     	 参数1：rdlvl_gate_delay(0xa70~0xa90)采样时机到选通信号上升沿的延迟
 * 		 参数2：rdlvl_(dqsp_)delay(0xa30~0xa50)Read Leveling时从第一个0到1延迟单元数	
 * 		 参数3：rdlvl_dqsn_delay(0x240~0x270)读数据返回时，DQSn的相位延迟	
 * 		 参数4：wrlvl_delay(0xaf0~0xb10)控制写DQS经DLL的延迟数	
 * 		 参数5：wrlvl_dq_delay(0x210~0x230)控制写数据DQ与DQS之间的相位关系
 * 		 参数6：phy_ctrl_0_gate(0x320~0x360)读采样延时0和1，用于控制读DQS采样采样窗口打开时机	
 * 参数：t3（选择对哪个DQ字节进行操作, 0xf将对所有字节进行操作）t4（选择哪个寄存器进行操作, 1~6）
 *       t5（修改的目标值）
 * 返回：无
 * 寄存器使用：s7（返回地址）t7（MC配置基址）
 */
	.global		ddr_param_modify
	.ent		ddr_param_modify	
ddr_param_modify:
	move		s7, ra									//t8, t9已使用	

#ifdef DEBUG_MODIFY_PARAM
	MP_DEBUG("Modify param operation: t3 = 0x")
	move	a0, t3
	bal		serial_puthex
	nop
	MP_DEBUG(", t4 = 0x")
	move	a0, t4
	bal		serial_puthex
	nop
	MP_DEBUG(", t5 = 0x")
	move	a0, t5
	bal		serial_puthex
	nop
	MP_DEBUG("\r\n")
#endif

	bal			prepare_modify_runtime	
	nop

	dli			a0, 0x01
	beq			t4, a0, 1f
	daddiu		a0, a0, 1
	beq			t4, a0, 2f
	daddiu		a0, a0, 1
	beq			t4, a0, 3f
	daddiu		a0, a0, 1
	beq			t4, a0, 4f
	daddiu		a0, a0, 1
	beq			t4, a0, 5f
	daddiu		a0, a0, 1
	beq			t4, a0, 6f
	nop
	
	MP_DEBUG("Unsupported operation!!!\r\n")
	b			88f
	nop

1:
	dli			a1, 0x00
	beq			t3, a1, 10f
	daddiu		a1, a1, 1
	beq			t3, a1, 11f
	daddiu		a1, a1, 1
	beq			t3, a1, 12f
	daddiu		a1, a1, 1
	beq			t3, a1, 13f
	daddiu		a1, a1, 1
	beq			t3, a1, 14f
	daddiu		a1, a1, 1
	beq			t3, a1, 15f
	daddiu		a1, a1, 1
	beq			t3, a1, 16f
	daddiu		a1, a1, 1
	beq			t3, a1, 17f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

10:
	ld			a0, CONF_CTL_167_REG(t7)
	and			a0, RDLVL_GATE_DELAY_0_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_167_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
11:
	ld			a0, CONF_CTL_168_REG(t7)
	and			a0, RDLVL_GATE_DELAY_1_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_168_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
12:
	ld			a0, CONF_CTL_168_REG(t7)
	and			a0, RDLVL_GATE_DELAY_2_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_168_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
13:
	ld			a0, CONF_CTL_168_REG(t7)
	and			a0, RDLVL_GATE_DELAY_3_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_168_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
14:
	ld			a0, CONF_CTL_168_REG(t7)
	and			a0, RDLVL_GATE_DELAY_4_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_168_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
15:
	ld			a0, CONF_CTL_169_REG(t7)
	and			a0, RDLVL_GATE_DELAY_5_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_169_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
16:
	ld			a0, CONF_CTL_169_REG(t7)
	and			a0, RDLVL_GATE_DELAY_6_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_169_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
17:
	ld			a0, CONF_CTL_169_REG(t7)
	and			a0, RDLVL_GATE_DELAY_7_MASK
	dsll		a2, t5, RDLVL_GATE_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_169_REG(t7)

	b			88f
	nop

2:
	dli			a1, 0x00
	beq			t3, a1, 20f
	daddiu		a1, a1, 1
	beq			t3, a1, 21f
	daddiu		a1, a1, 1
	beq			t3, a1, 22f
	daddiu		a1, a1, 1
	beq			t3, a1, 23f
	daddiu		a1, a1, 1
	beq			t3, a1, 24f
	daddiu		a1, a1, 1
	beq			t3, a1, 25f
	daddiu		a1, a1, 1
	beq			t3, a1, 26f
	daddiu		a1, a1, 1
	beq			t3, a1, 27f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

20:
	ld			a0, CONF_CTL_163_REG(t7)
	and			a0, RDLVL_DELAY_0_MASK
	dsll		a2, t5, RDLVL_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_163_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
21:
	ld			a0, CONF_CTL_163_REG(t7)
	and			a0, RDLVL_DELAY_1_MASK
	dsll		a2, t5, RDLVL_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_163_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
22:
	ld			a0, CONF_CTL_163_REG(t7)
	and			a0, RDLVL_DELAY_2_MASK
	dsll		a2, t5, RDLVL_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_163_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
23:
	ld			a0, CONF_CTL_164_REG(t7)
	and			a0, RDLVL_DELAY_3_MASK
	dsll		a2, t5, RDLVL_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_164_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
24:
	ld			a0, CONF_CTL_164_REG(t7)
	and			a0, RDLVL_DELAY_4_MASK
	dsll		a2, t5, RDLVL_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_164_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
25:
	ld			a0, CONF_CTL_164_REG(t7)
	and			a0, RDLVL_DELAY_5_MASK
	dsll		a2, t5, RDLVL_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_164_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
26:
	ld			a0, CONF_CTL_164_REG(t7)
	and			a0, RDLVL_DELAY_6_MASK
	dsll		a2, t5, RDLVL_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_164_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
27:
	ld			a0, CONF_CTL_165_REG(t7)
	and			a0, RDLVL_DELAY_7_MASK
	dsll		a2, t5, RDLVL_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_165_REG(t7)

	b			88f
	nop

3:
	dli			a1, 0x00
	beq			t3, a1, 30f
	daddiu		a1, a1, 1
	beq			t3, a1, 31f
	daddiu		a1, a1, 1
	beq			t3, a1, 32f
	daddiu		a1, a1, 1
	beq			t3, a1, 33f
	daddiu		a1, a1, 1
	beq			t3, a1, 34f
	daddiu		a1, a1, 1
	beq			t3, a1, 35f
	daddiu		a1, a1, 1
	beq			t3, a1, 36f
	daddiu		a1, a1, 1
	beq			t3, a1, 37f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

30:
	ld			a0, CONF_CTL_36_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_0_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_36_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
31:
	ld			a0, CONF_CTL_36_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_1_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_36_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
32:
	ld			a0, CONF_CTL_37_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_2_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_37_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
33:
	ld			a0, CONF_CTL_37_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_3_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_37_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
34:
	ld			a0, CONF_CTL_38_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_4_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_38_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
35:
	ld			a0, CONF_CTL_38_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_5_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_38_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
36:
	ld			a0, CONF_CTL_39_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_6_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_39_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
37:
	ld			a0, CONF_CTL_39_REG(t7)
	and			a0, RDLVL_DQSN_DELAY_7_MASK
	dsll		a2, t5, RDLVL_DQSN_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_39_REG(t7)

	b			88f
	nop

4:
	dli			a1, 0x00
	beq			t3, a1, 40f
	daddiu		a1, a1, 1
	beq			t3, a1, 41f
	daddiu		a1, a1, 1
	beq			t3, a1, 42f
	daddiu		a1, a1, 1
	beq			t3, a1, 43f
	daddiu		a1, a1, 1
	beq			t3, a1, 44f
	daddiu		a1, a1, 1
	beq			t3, a1, 45f
	daddiu		a1, a1, 1
	beq			t3, a1, 46f
	daddiu		a1, a1, 1
	beq			t3, a1, 47f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

40:
	ld			a0, CONF_CTL_175_REG(t7)
	and			a0, WRLVL_DELAY_0_MASK
	dsll		a2, t5, WRLVL_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_175_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_45_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_45_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
41:
	ld			a0, CONF_CTL_175_REG(t7)
	and			a0, WRLVL_DELAY_1_MASK
	dsll		a2, t5, WRLVL_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_175_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_46_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_46_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
42:
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_2_MASK
	dsll		a2, t5, WRLVL_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_46_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_46_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
43:
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_3_MASK
	dsll		a2, t5, WRLVL_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_47_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_47_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
44:
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_4_MASK
	dsll		a2, t5, WRLVL_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_47_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_47_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
45:
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_5_MASK
	dsll		a2, t5, WRLVL_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_48_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_48_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
46:
	ld			a0, CONF_CTL_177_REG(t7)
	and			a0, WRLVL_DELAY_6_MASK
	dsll		a2, t5, WRLVL_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_177_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_48_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_48_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
47:
	ld			a0, CONF_CTL_177_REG(t7)
	and			a0, WRLVL_DELAY_7_MASK
	dsll		a2, t5, WRLVL_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_177_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 增加写操作延迟时钟周期 */
	dli			a0, DQSDQ_OUT_WINDOW_VALUE		

	dli			a1, WRLVL_HALF_CLK_VALUE
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_00				//t5 < 0x40
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x60
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_40				//0x40 <= t5 < 0x60
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a1, 0x80
	bge			t5, a1, 1f
	nop
	dli			a2, PHY_DQSDQ_INC_VALUE_60				//0x60 <= t5 < 0x80
	daddu		a3, a0, a2
	b			2f
	nop	
1:
	dli			a2, PHY_DQSDQ_INC_VALUE_80				//0x80 < t5 
	daddu		a3, a0, a2
2:
	ld			a0, CONF_CTL_49_REG(t7)	
	dli			a1, PHY_CTRL_0_CLK_ADD_MASK
	dsll		a1, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	not			a1, a1
	and			a0, a1
	dsrl		a3, 16
	dsll		a3, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_CLK_ADD_SHIFT
	or			a0, a3
	sd			a0, CONF_CTL_49_REG(t7)
#endif //WRLVL_HALF_CLK_CLEAR

	b			88f
	nop

5:
	dli			a1, 0x00
	beq			t3, a1, 50f
	daddiu		a1, a1, 1
	beq			t3, a1, 51f
	daddiu		a1, a1, 1
	beq			t3, a1, 52f
	daddiu		a1, a1, 1
	beq			t3, a1, 53f
	daddiu		a1, a1, 1
	beq			t3, a1, 54f
	daddiu		a1, a1, 1
	beq			t3, a1, 55f
	daddiu		a1, a1, 1
	beq			t3, a1, 56f
	daddiu		a1, a1, 1
	beq			t3, a1, 57f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

50:
	ld			a0, CONF_CTL_31_REG(t7)
	and			a0, WRLVL_DQ_DELAY_0_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_31_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
51:
	ld			a0, CONF_CTL_32_REG(t7)
	and			a0, WRLVL_DQ_DELAY_1_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_32_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
52:
	ld			a0, CONF_CTL_32_REG(t7)
	and			a0, WRLVL_DQ_DELAY_2_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_32_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
53:
	ld			a0, CONF_CTL_33_REG(t7)
	and			a0, WRLVL_DQ_DELAY_3_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_33_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
54:
	ld			a0, CONF_CTL_33_REG(t7)
	and			a0, WRLVL_DQ_DELAY_4_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_33_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
55:
	ld			a0, CONF_CTL_34_REG(t7)
	and			a0, WRLVL_DQ_DELAY_5_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_34_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
56:
	ld			a0, CONF_CTL_34_REG(t7)
	and			a0, WRLVL_DQ_DELAY_6_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_34_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
57:
	ld			a0, CONF_CTL_35_REG(t7)
	and			a0, WRLVL_DQ_DELAY_7_MASK
	dsll		a2, t5, WRLVL_DQ_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_35_REG(t7)

	b			88f
	nop

6:
	dli			a1, 0x00
	beq			t3, a1, 60f
	daddiu		a1, a1, 1
	beq			t3, a1, 61f
	daddiu		a1, a1, 1
	beq			t3, a1, 62f
	daddiu		a1, a1, 1
	beq			t3, a1, 63f
	daddiu		a1, a1, 1
	beq			t3, a1, 64f
	daddiu		a1, a1, 1
	beq			t3, a1, 65f
	daddiu		a1, a1, 1
	beq			t3, a1, 66f
	daddiu		a1, a1, 1
	beq			t3, a1, 67f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

60:
	ld			a0, CONF_CTL_50_REG(t7)
	and			a0, PHY_CTRL_1_GATE_0_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_50_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_50_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_0_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_0_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_50_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
61:
	ld			a0, CONF_CTL_50_REG(t7)
	and			a0, PHY_CTRL_1_GATE_1_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_50_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_50_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_1_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_1_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_50_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop	
62:
	ld			a0, CONF_CTL_51_REG(t7)
	and			a0, PHY_CTRL_1_GATE_2_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_51_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_51_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_2_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_2_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_51_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
63:
	ld			a0, CONF_CTL_51_REG(t7)
	and			a0, PHY_CTRL_1_GATE_3_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_51_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_51_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_3_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_3_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_51_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
64:
	ld			a0, CONF_CTL_52_REG(t7)
	and			a0, PHY_CTRL_1_GATE_4_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_52_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_52_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_4_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_4_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_52_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
65:
	ld			a0, CONF_CTL_52_REG(t7)
	and			a0, PHY_CTRL_1_GATE_5_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_52_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_52_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_5_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_5_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_52_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
66:
	ld			a0, CONF_CTL_53_REG(t7)
	and			a0, PHY_CTRL_1_GATE_6_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_53_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_53_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_6_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_6_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_53_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
67:
	ld			a0, CONF_CTL_53_REG(t7)
	and			a0, PHY_CTRL_1_GATE_7_MASK
	dsll		a2, t5, PHY_CTRL_REG_1_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_53_REG(t7)
	/* 调整读ODT */	
	ld			a0, CONF_CTL_53_REG(t7)
	and			a0, PHY_CTRL_1_RD_ODT_7_MASK
	dli			a2, CPU_ODT_BASE_VALUE
	beqz		t5, 1f
	nop
	daddu		a2, CPU_ODT_INC_VALUE
1:
	dsll		a2, PHY_CTRL_REG_1_7_OFFSET + PHY_CTRL_1_RD_ODT_SHIFT
	or			a0, a2
	sd			a0, CONF_CTL_53_REG(t7)

	b			88f
	nop

88:
	bal			release_modify_runtime
	nop

	MP_DEBUG("param modify done!\r\n")

	jr			s7
	nop
	.end	ddr_param_modify

/*
 * 描述：DDR运行修改MC读数据返回采样完成时机参数phy0_gate(0x2d0~0x310)，从内部时钟域采样的延迟
 *       如果原始值大于等于4，则将值设置为3，否则设置为4
 * 参数：t3（选择对哪个DQ字节进行操作, 0xf将对所有字节进行操作）
 * 返回：无
 * 寄存器使用：s7（返回地址）t7（MC配置基址）
 */
	.global	ddr_modify_pop_delay_alter
	.ent	ddr_modify_pop_delay_alter	
ddr_modify_pop_delay_alter:
	move		s7, ra

#ifdef DEBUG_MODIFY_PARAM
	MP_DEBUG("Modify pop delay: t3 = 0x")
	move	a0, t3
	bal		serial_puthex
	nop
	MP_DEBUG("\r\n")
#endif

	bal			prepare_modify_runtime	
	nop

	dli			a1, 0x00
	beq			t3, a1, 10f
	daddiu		a1, a1, 1
	beq			t3, a1, 11f
	daddiu		a1, a1, 1
	beq			t3, a1, 12f
	daddiu		a1, a1, 1
	beq			t3, a1, 13f
	daddiu		a1, a1, 1
	beq			t3, a1, 14f
	daddiu		a1, a1, 1
	beq			t3, a1, 15f
	daddiu		a1, a1, 1
	beq			t3, a1, 16f
	daddiu		a1, a1, 1
	beq			t3, a1, 17f
	nop
	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop

10:
	ld			a1, CONF_CTL_45_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_45_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
11:
	ld			a1, CONF_CTL_46_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_46_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
12:
	ld			a1, CONF_CTL_46_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_46_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
13:
	ld			a1, CONF_CTL_47_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_47_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
14:
	ld			a1, CONF_CTL_47_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_47_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
15:
	ld			a1, CONF_CTL_48_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_48_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
16:
	ld			a1, CONF_CTL_48_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_48_REG(t7)

	dli			a1, 0x0f
	bne			t3, a1, 88f
	nop
17:
	ld			a1, CONF_CTL_49_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	dli			a0, 0x04
	blt			a2, a0, 1f
	nop
	daddu		a0, -1	
1:	
	dli			a2, PHY_CTRL_0_POP_DELAY_MASK	
	dsll		a2, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	not			a2, a2
	and			a1, a2
	dsll		a0, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_49_REG(t7)

	b			88f
	nop

88:
	bal			release_modify_runtime
	nop

	jr			s7
	nop
	.end  	ddr_modify_pop_delay_alter

/*
 * 描述：当获取通过测试的窗口失败时，使用该函数添加PAD补偿，以重新进行校准测试
 		 并以PAD值MAX，MIN，MID固定顺序进行补偿，当当前PAD值为MID时将返回失败
 * 参数：
 * 返回：v0（0操作成功，1操作失败）
 * 寄存器使用：t7（MC配置基地址），s7（返回地址），t2（临时保存结果）
 * 
 */
	.global	ddr_modify_pad_comp
	.ent	ddr_modify_pad_comp
ddr_modify_pad_comp:
	move		s7, ra	
	bal			prepare_modify_runtime
	nop

	ld			a1, CONF_CTL_45_REG(t7)	
	dsrl		a2, a1, PAD_CTRL_REG_0_OFFSET + PAD_CTRL_COMP_SHIFT
	and			a2, PAD_CTRL_COMP_MASK
	and			a2, 0x0f
	
	beqz		a2, pad_comp_max							//max
	nop
	
	dli			a3, 0x0f
	beq			a2, a3, pad_comp_min						//min
	nop
	
	dli			t2, 0x01
	b			88f
	nop	
	
pad_comp_max:
	dli			a2, PAD_CTRL_COMP_MASK
	dsll		a2, PAD_CTRL_REG_0_OFFSET + PAD_CTRL_COMP_SHIFT
	not			a2, a2
	and			a1, a2
	dli			a0, 0x0f
	dsll		a0, PAD_CTRL_REG_0_OFFSET + PAD_CTRL_COMP_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_45_REG(t7)

	move		t2, zero
	b			88f
	nop

pad_comp_min:
	dli			a2, PAD_CTRL_COMP_MASK
	dsll		a2, PAD_CTRL_REG_0_OFFSET + PAD_CTRL_COMP_SHIFT
	not			a2, a2
	and			a1, a2
	dli			a0, 0x1e
	dsll		a0, PAD_CTRL_REG_0_OFFSET + PAD_CTRL_COMP_SHIFT
	or			a1, a0
	sd			a1, CONF_CTL_45_REG(t7)

	move		t2, zero

88:
	bal			release_modify_runtime
	nop

#ifdef DEBUG_MODIFY_PARAM
	MP_DEBUG("try pad compensation, result: v0 = 0x")
	move		a0, t2
	bal			serial_puthex
	nop	
	MP_DEBUG("\r\n")
#endif

	move		v0, t2

	jr			s7
	nop
	.end	ddr_modify_pad_comp
	
/*
 * 描述：将计算得到的MID值写入寄存器，并根据该值是否大于0x40来决定是否添加半个时钟周期
 * 参数：t3保存8个字节计算得出的MID值
 * 返回：
 * 寄存器使用：t7（MC配置基地址），s7（返回地址）
 * 
 */
	.global	ddr_write_wrlvl_delay
	.ent	ddr_write_wrlvl_delay
ddr_write_wrlvl_delay:
	move		s7, ra
	bal			prepare_modify_runtime
	nop

	//byte7
	ld			a0, CONF_CTL_177_REG(t7)
	and			a0, WRLVL_DELAY_7_MASK
	dsrl		a1, t3, 56
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_7_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_177_REG(t7)
	//byte6
	ld			a0, CONF_CTL_177_REG(t7)
	and			a0, WRLVL_DELAY_6_MASK
	dsrl		a1, t3, 48
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_6_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_177_REG(t7)
	//byte5
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_5_MASK
	dsrl		a1, t3, 40
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_5_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)
	//byte4
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_4_MASK
	dsrl		a1, t3, 32
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_4_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)
	//byte3
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_3_MASK
	dsrl		a1, t3, 24
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_3_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)
	//byte2
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, WRLVL_DELAY_2_MASK
	dsrl		a1, t3, 16
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_2_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_176_REG(t7)
	//byte1
	ld			a0, CONF_CTL_175_REG(t7)
	and			a0, WRLVL_DELAY_1_MASK
	dsrl		a1, t3, 8
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_1_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_175_REG(t7)
	//byte0
	ld			a0, CONF_CTL_175_REG(t7)
	and			a0, WRLVL_DELAY_0_MASK
	dsrl		a1, t3, 0
	and			a1, 0x7f
	dsll		a2, a1, WRLVL_DELAY_0_OFFSET
	or			a0, a2
	sd			a0, CONF_CTL_175_REG(t7)

#ifdef WRLVL_HALF_CLK_CLEAR
	/* 曾加半时钟周期 */	
	dli			a0, WRLVL_HALF_CLK_VALUE				

	//byte7
	dsrl		a1, t3, 56
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_49_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_49_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_49_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_49_REG(t7)

	//byte6
	dsrl		a1, t3, 48
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_48_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_48_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_48_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_48_REG(t7)

	//byte5
	dsrl		a1, t3, 40
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_48_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_48_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_48_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_48_REG(t7)
2:
	//byte4
	dsrl		a1, t3, 32
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_47_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_47_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_47_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_47_REG(t7)
2:
	//byte3
	dsrl		a1, t3, 24
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_47_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_47_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_47_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_47_REG(t7)
2:
	//byte2
	dsrl		a1, t3, 16
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_46_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_46_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_46_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_46_REG(t7)
2:
	//byte1
	dsrl		a1, t3, 8
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_46_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_46_REG(t7)
	b			2f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_46_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_46_REG(t7)
2:
	//byte0
	dsrl		a1, t3, 0
	and			a1, 0x7f
	blt			a1, a0, 1f
	nop
	//set halt clk	
	ld			a1, CONF_CTL_45_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	or			a1, a2
	sd			a1, CONF_CTL_45_REG(t7)
	b			88f
	nop	
1:	//clear halt clk
	ld			a1, CONF_CTL_45_REG(t7)	
	dli			a2, PHY_CTRL_0_CLK_WRADD_MASK
	dsll		a2, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_CLK_WRADD_SHIFT
	not			a2, a2
	and			a1, a2
	sd			a1, CONF_CTL_45_REG(t7)
#endif //WRLVL_HALF_CLK_VALUE

88:
	bal			release_modify_runtime
	nop

	jr			s7
	nop
	.end	ddr_write_wrlvl_delay

/*
 * 描述：DDR运行修改读数据有效时机，从FIFO中收集数据返回控制器的延迟(0x360)。
 * 		 如果其他任一byte（0~7）的phy0_pop_delay参数值大于等于4，则将值设置为7，
 *		 否则不进行任何修改。将byte4（DDR3 DIMM）的phy0配置、phy1配置、rdlvl_gete、
 *		 rdlvl_dqs_p、rdlvl_dqs_n、wrlvl_delay、wrlvl_dq参数全部写入到byte8对应寄存器中。
 * 参数：无
 * 返回：无
 * 寄存器使用：s7（返回地址）t7（MC配置基址）
 */
	.global	ddr_write_slice_8_param
	.ent	ddr_write_slice_8_param
ddr_write_slice_8_param:
	move		s7, ra

	bal			prepare_modify_runtime	
	nop

	dli			a0, 0x04

	ld			a1, CONF_CTL_45_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_0_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_46_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_1_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_46_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_2_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_47_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_3_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_47_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_4_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_48_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_5_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_48_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_6_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	ld			a1, CONF_CTL_49_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_7_OFFSET + PHY_CTRL_0_POP_DELAY_SHIFT
	and			a2, PHY_CTRL_0_POP_DELAY_MASK
	blt			a2, a0, 1f
	nop

	/* 如果byte0~7有字节的pop_delay参数大于等于4，则需要将FIFO收集延迟值设置为7 */
	ld			a1, CONF_CTL_54_REG(t7)	
	dli			a0, PHY_CTRL_2_POP_DELAY_MASK	
	dsll		a0, PHY_CTRL_2_POP_DELAY_SHIFT + PHY_CTRL_REG_2_OFFSET
	not			a0, a0
	and			a1, a0
	dli			a0, 0x07
	dsll		a0, PHY_CTRL_2_POP_DELAY_SHIFT + PHY_CTRL_REG_2_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_54_REG(t7)

1:
	/* 将byte4的phy0寄存器配置设置到byte8 */
	ld			a1, CONF_CTL_47_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_0_4_OFFSET 
	and			a2, PHY_CTRL_REG_0_MASK

	ld			a1, CONF_CTL_49_REG(t7)
	dli			a0, PHY_CTRL_REG_0_MASK
	dsll		a0, PHY_CTRL_REG_0_8_OFFSET 
	not			a0, a0
	and			a1, a0
	dsll		a2, PHY_CTRL_REG_0_8_OFFSET	
	or			a1, a2
	sd			a1, CONF_CTL_49_REG(t7)	
		
	/* 将byte4的phy1寄存器配置设置到byte8 */
	ld			a1, CONF_CTL_52_REG(t7)
	dsrl		a2, a1, PHY_CTRL_REG_1_4_OFFSET 
	and			a2, PHY_CTRL_REG_1_MASK

	ld			a1, CONF_CTL_54_REG(t7)
	dli			a0, PHY_CTRL_REG_1_MASK
	dsll		a0, PHY_CTRL_REG_1_8_OFFSET 
	not			a0, a0
	and			a1, a0
	dsll		a2, PHY_CTRL_REG_1_8_OFFSET	
	or			a1, a2
	sd			a1, CONF_CTL_54_REG(t7)	

	/* 将byte4的rdlvl_gate参数设置到byte8 */
	ld			a0, CONF_CTL_168_REG(t7)
	and			a0, ~(RDLVL_GATE_DELAY_4_MASK)
	dsrl		a0, RDLVL_GATE_DELAY_4_OFFSET

	ld			a1, CONF_CTL_169_REG(t7)	
	and			a1, RDLVL_GATE_DELAY_8_MASK
	dsll		a0, RDLVL_GATE_DELAY_8_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_169_REG(t7)

	/* 将byte4的rdlvl_dqs_p参数设置到byte8 */
	ld			a0, CONF_CTL_164_REG(t7)
	and			a0, ~(RDLVL_DELAY_4_MASK)
	dsrl		a0, RDLVL_DELAY_4_OFFSET

	ld			a1, CONF_CTL_165_REG(t7)	
	and			a1, RDLVL_DELAY_8_MASK
	dsll		a0, RDLVL_DELAY_8_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_165_REG(t7)

	/* 将byte4的rdlvl_dqs_n参数设置到byte8 */
	ld			a0, CONF_CTL_38_REG(t7)
	and			a0, ~(RDLVL_DQSN_DELAY_4_MASK)
	dsrl		a0, RDLVL_DQSN_DELAY_4_OFFSET

	ld			a1, CONF_CTL_40_REG(t7)	
	and			a1, RDLVL_DQSN_DELAY_8_MASK
	dsll		a0, RDLVL_DQSN_DELAY_8_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_40_REG(t7)

	/* 将byte4的wrlvl_delay参数设置到byte8 */
	ld			a0, CONF_CTL_176_REG(t7)
	and			a0, ~(WRLVL_DELAY_4_MASK)
	dsrl		a0, WRLVL_DELAY_4_OFFSET

	ld			a1, CONF_CTL_177_REG(t7)	
	and			a1, WRLVL_DELAY_8_MASK
	dsll		a0, WRLVL_DELAY_8_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_177_REG(t7)

	/* 将byte4的wrlvl_dq参数设置到byte8 */
	ld			a0, CONF_CTL_33_REG(t7)
	and			a0, ~(WRLVL_DQ_DELAY_4_MASK)
	dsrl		a0, WRLVL_DQ_DELAY_4_OFFSET

	ld			a1, CONF_CTL_35_REG(t7)	
	and			a1, WRLVL_DQ_DELAY_8_MASK
	dsll		a0, WRLVL_DQ_DELAY_8_OFFSET
	or			a1, a0
	sd			a1, CONF_CTL_35_REG(t7)

	bal			release_modify_runtime
	nop

	jr			s7
	nop
	.end  	ddr_write_slice_8_param


/*
 * 描述：打开DDR运行时MC参数修改准备过程
 * 
 * 寄存器使用：t7（MC配置基址）
 *
 */
LEAF(prepare_modify_runtime)
	sync												//1.禁用地址窗口
	sync
	sync
	sync
	sync
	sync
	sync
	GET_LEVEL_XBAR_REG_TO_a0
	ld			a1, XBAR_WIN_MMAP_OFFSET(a0)	
	dli			a2, LEVEL_XBAR_MMAP
	not			a2, a2
	and			a1, a2
	sd			a1, XBAR_WIN_MMAP_OFFSET(a0)	
	sync
	sync
	sync
	sync
	sync
	sync
	sync

	GET_NODE_ID_TO_a1									//2.打开配置空间	
	dsll		a1, 44
	or			a1, DDR_CONFIG_SPACE_REG	
	lw			a0, 0x00(a1)
	or			a0, DDR_ENABLE_CONFIG_SPACE
	sw			a0, 0x00(a1)	
	sync
			
	ld			a0, CONF_CTL_03_REG(t7)					//3.进入自刷新模式
	or			a0, ENTER_SREFRESH	
	sd			a0, CONF_CTL_03_REG(t7)	
	sync
	
	//wait a while
	dli			a0, 0x400
1:
	bnez		a0, 1b
	daddiu		a0, -1
	
	ld			a0, CONF_CTL_03_REG(t7)					//4.禁止DDR初始化	
	dli			a1, START_DDR
	not			a1, a1
	and			a0, a1
	sd			a0, CONF_CTL_03_REG(t7)	
	
	ld			a0, CONF_CTL_54_REG(t7)					//5.清楚读FIFO
	or			a0, CLEAR_READ_FIFO
	sd			a0, CONF_CTL_54_REG(t7)	

	jr			ra
	nop
END(prepare_modify_runtime)

/*
 * 描述：打开DDR运行时MC参数修改准备过程
 * 
 * 寄存器使用：t7（MC配置基址）
 *
 */
LEAF(release_modify_runtime)
	sync

	ld			a0, CONF_CTL_54_REG(t7)					//1.读FIFO正常工作
	dli			a1, CLEAR_READ_FIFO
	not 		a1, a1
	and			a0, a1	
	sd			a0, CONF_CTL_54_REG(t7)	

	ld			a0, CONF_CTL_03_REG(t7)					//2.打开DDR初始化	
	or			a0, START_DDR
	sd			a0, CONF_CTL_03_REG(t7)	
	sync
		
1:
	ld			a0, CONF_CTL_01_REG(t7)					//3.等待DLL LOCK
	and			a0, DLL_LOCKED
	beqz		a0, 1b
	nop

	ld			a0, CONF_CTL_03_REG(t7)					//4.退出自刷新模式
	dli			a1, ENTER_SREFRESH	
	not			a1, a1
	and			a0, a1
	sd			a0, CONF_CTL_03_REG(t7)	
	sync

	//wait a while
	dli			a0, 0x400
1:
	bnez		a0, 1b
	daddiu		a0, -1
	
	GET_NODE_ID_TO_a1									//5.关闭配置空间
	dsll		a1, 44
	or			a1, DDR_CONFIG_SPACE_REG	
	lw			a0, 0x00(a1)
	and			a0, DDR_DISABLE_CONFIG_SPACE
	sw			a0, 0x00(a1)	
	sync

	sync												//6.打开地址窗口
	sync
	sync
	sync
	sync
	sync
	sync
	GET_LEVEL_XBAR_REG_TO_a0
	ld			a1, XBAR_WIN_MMAP_OFFSET(a0)	
	or			a1, LEVEL_XBAR_MMAP
	sd			a1, XBAR_WIN_MMAP_OFFSET(a0)	
	sync
	sync
	sync
	sync
	sync
	sync
	sync

	jr			ra
	nop
END(release_modify_runtime)

/*
 * 描述：打开内存配置空间
 * 
 */
LEAF(enable_ddr_config_space)
	GET_NODE_ID_TO_a1		
	dsll		a1, 44
	or			a1, DDR_CONFIG_SPACE_REG	
	lw			a0, 0x00(a1)
	or			a0, DDR_ENABLE_CONFIG_SPACE
	sw			a0, 0x00(a1)	
	sync 

	jr			ra
	nop
END(enable_ddr_config_space)

/*
 * 描述：关闭内存配置空间
 * 
 */
LEAF(disable_ddr_config_space)
	GET_NODE_ID_TO_a1		
	dsll		a1, 44
	or			a1, DDR_CONFIG_SPACE_REG	
	lw			a0, 0x00(a1)
	and			a0, DDR_DISABLE_CONFIG_SPACE
	sw			a0, 0x00(a1)	
	sync

	jr			ra
	nop
END(disable_ddr_config_space)
