
/**
 * Copyright(c) 2015-7-13 Shangwen Wu	
 *
 * DDR参数配置寄存器定义
 * 
 */
#ifndef __DDR_PARAM_DEFINE__
#define __DDR_PARAM_DEFINE__

#define LS2H_DDR_PARAM_NUM					180						/* 龙芯DDR配置寄存器个数 */

#define CONF_CTL_01_REG						0x10						
#define DLL_LOCKED							0x0000000000000001		/* DLL锁定 */
#define EIGHT_BANK_MODE_OFFSET				32
#define EIGHT_BANK_MODE_MASK				0xfffffffeffffffff	

#define CONF_CTL_03_REG						0x30						
#define	START_DDR							0x0000010000000000		/* 开始DDR初始化 */
#define ENTER_SREFRESH						0x0000000100000000		/* DDR进入自刷新模式 */
#define REDUC_MASK							0xffffffffffffffef		/* 32位内存通道模式 */
#define REDUC_OFFSET						8

#define CONF_CTL_04_REG						0x40						
#define CTRL_RAW_MASK						0xfffcffffffffffff		/* ECC模式 */
#define CTRL_RAW_OFFSET						48

#define CONF_CTL_05_REG						0x50
#define ROWCOL_SIZE_MASK					0xffffffff00ff00ff
#define ADDR_PINS_OFFSET					8
#define COLNUM_SIZE_OFFSET					24
#define ROW_PINS_MAX						15
#define COL_PINS_MAX						14

#define CONF_CTL_07_REG						0x70
#define CS_MAP_MASK							0xfffffffffff0ffff
#define CS_MAP_OFFSET						16

#define CONF_CTL_08_REG						0x80

#define CONF_CTL_10_REG						0xa0
#define TFAW_MASK							0xffffffffffffffe0		/* tFAW时序参数 */
#define TFAW_OFFSET							0

#define CONF_CTL_12_REG						0xc0
#define TRFC_MASK							0xffff00ffffffffff
#define TRFC_OFFSET							40
#define TRFC_MARGIN							0x08

#define CONF_CTL_20_REG						0x140
#define TXSNR_MASK							0xffffffffffff0000
#define TXSNR_OFFSET						0

#define CONF_CTL_31_REG						0x1f0
#define WRLVL_DQ_DELAY_0_OFFSET				48
#define WRLVL_DQ_DELAY_0_MASK				0xff00ffffffffffff

#define CONF_CTL_32_REG						0x200
#define WRLVL_DQ_DELAY_2_OFFSET				48
#define WRLVL_DQ_DELAY_2_MASK				0xff00ffffffffffff
#define WRLVL_DQ_DELAY_1_OFFSET				16
#define WRLVL_DQ_DELAY_1_MASK				0xffffffffff00ffff

#define CONF_CTL_33_REG						0x210
#define WRLVL_DQ_DELAY_4_OFFSET				48
#define WRLVL_DQ_DELAY_4_MASK				0xff00ffffffffffff
#define WRLVL_DQ_DELAY_3_OFFSET				16
#define WRLVL_DQ_DELAY_3_MASK				0xffffffffff00ffff

#define CONF_CTL_34_REG						0x220
#define WRLVL_DQ_DELAY_6_OFFSET				48
#define WRLVL_DQ_DELAY_6_MASK				0xff00ffffffffffff
#define WRLVL_DQ_DELAY_5_OFFSET				16
#define WRLVL_DQ_DELAY_5_MASK				0xffffffffff00ffff

#define CONF_CTL_35_REG						0x230
#define WRLVL_DQ_DELAY_8_OFFSET				48
#define WRLVL_DQ_DELAY_8_MASK				0xff00ffffffffffff
#define WRLVL_DQ_DELAY_7_OFFSET				16
#define WRLVL_DQ_DELAY_7_MASK				0xffffffffff00ffff

#define CONF_CTL_36_REG						0x240
#define RDLVL_DQSN_DELAY_1_OFFSET			40
#define RDLVL_DQSN_DELAY_1_MASK				0xffff00ffffffffff
#define RDLVL_DQSN_DELAY_0_OFFSET			8
#define RDLVL_DQSN_DELAY_0_MASK				0xffffffffffff00ff

#define CONF_CTL_37_REG						0x250
#define RDLVL_DQSN_DELAY_3_OFFSET			40
#define RDLVL_DQSN_DELAY_3_MASK				0xffff00ffffffffff
#define RDLVL_DQSN_DELAY_2_OFFSET			8
#define RDLVL_DQSN_DELAY_2_MASK				0xffffffffffff00ff

#define CONF_CTL_38_REG						0x260
#define RDLVL_DQSN_DELAY_5_OFFSET			40
#define RDLVL_DQSN_DELAY_5_MASK				0xffff00ffffffffff
#define RDLVL_DQSN_DELAY_4_OFFSET			8
#define RDLVL_DQSN_DELAY_4_MASK				0xffffffffffff00ff

#define CONF_CTL_39_REG						0x270
#define RDLVL_DQSN_DELAY_7_OFFSET			40
#define RDLVL_DQSN_DELAY_7_MASK				0xffff00ffffffffff
#define RDLVL_DQSN_DELAY_6_OFFSET			8
#define RDLVL_DQSN_DELAY_6_MASK				0xffffffffffff00ff

#define CONF_CTL_40_REG						0x280
#define RDLVL_DQSN_DELAY_8_OFFSET			8
#define RDLVL_DQSN_DELAY_8_MASK				0xffffffffffff00ff

#define CONF_CTL_45_REG						0x2d0
#define PHY_CTRL_REG_0_0_OFFSET				32
#define PAD_CTRL_REG_0_OFFSET				0

#define CONF_CTL_46_REG						0x2e0
#define PHY_CTRL_REG_0_2_OFFSET				32
#define PHY_CTRL_REG_0_1_OFFSET				0

#define CONF_CTL_47_REG						0x2f0
#define PHY_CTRL_REG_0_4_OFFSET				32
#define PHY_CTRL_REG_0_3_OFFSET				0

#define CONF_CTL_48_REG						0x300
#define PHY_CTRL_REG_0_6_OFFSET				32
#define PHY_CTRL_REG_0_5_OFFSET				0

#define CONF_CTL_49_REG						0x310
#define PHY_CTRL_REG_0_8_OFFSET				32
#define PHY_CTRL_REG_0_7_OFFSET				0

#define PHY_CTRL_REG_0_MASK					0xffffffff
#define PHY_CTRL_0_CLK_ADD_MASK				0xb
#define PHY_CTRL_0_CLK_ADD_SHIFT			16
#define PHY_CTRL_0_CLK_WRADD_MASK			0x3
#define PHY_CTRL_0_CLK_WRADD_SHIFT			16
#define PHY_CTRL_0_POP_DELAY_MASK			0x7
#define PHY_CTRL_0_POP_DELAY_SHIFT			24
#define PAD_CTRL_COMP_SHIFT					18
#define PAD_CTRL_COMP_MASK					0xff

#define CONF_CTL_50_REG						0x320
#define PHY_CTRL_REG_1_1_OFFSET				32
#define PHY_CTRL_1_GATE_1_MASK				0xfffff000ffffffff
#define PHY_CTRL_1_RD_ODT_1_MASK			0x00ffffffffffffff
#define PHY_CTRL_REG_1_0_OFFSET				0
#define PHY_CTRL_1_GATE_0_MASK				0xfffffffffffff000
#define PHY_CTRL_1_RD_ODT_0_MASK			0xffffffff00ffffff

#define CONF_CTL_51_REG						0x330
#define PHY_CTRL_REG_1_3_OFFSET				32
#define PHY_CTRL_1_GATE_3_MASK				0xfffff000ffffffff
#define PHY_CTRL_1_RD_ODT_3_MASK			0x00ffffffffffffff
#define PHY_CTRL_REG_1_2_OFFSET				0
#define PHY_CTRL_1_GATE_2_MASK				0xfffffffffffff000
#define PHY_CTRL_1_RD_ODT_2_MASK			0xffffffff00ffffff

#define CONF_CTL_52_REG						0x340
#define PHY_CTRL_REG_1_5_OFFSET				32
#define PHY_CTRL_1_GATE_5_MASK				0xfffff000ffffffff
#define PHY_CTRL_1_RD_ODT_5_MASK			0x00ffffffffffffff
#define PHY_CTRL_REG_1_4_OFFSET				0
#define PHY_CTRL_1_GATE_4_MASK				0xfffffffffffff000
#define PHY_CTRL_1_RD_ODT_4_MASK			0xffffffff00ffffff

#define CONF_CTL_53_REG						0x350
#define PHY_CTRL_REG_1_7_OFFSET				32
#define PHY_CTRL_1_GATE_7_MASK				0xfffff000ffffffff
#define PHY_CTRL_1_RD_ODT_7_MASK			0x00ffffffffffffff
#define PHY_CTRL_REG_1_6_OFFSET				0
#define PHY_CTRL_1_GATE_6_MASK				0xfffffffffffff000
#define PHY_CTRL_1_RD_ODT_6_MASK			0xffffffff00ffffff

#define CONF_CTL_54_REG						0x360
#define PHY_CTRL_REG_2_OFFSET				32
#define PHY_CTRL_2_POP_DELAY_SHIFT			13
#define PHY_CTRL_2_POP_DELAY_MASK			0x0f
#define PHY_CTRL_REG_1_8_OFFSET				0
#define PHY_CTRL_1_GATE_8_MASK				0xfffff000ffffffff
#define PHY_CTRL_1_RD_ODT_8_MASK			0x00ffffffffffffff
#define CLEAR_READ_FIFO						0x0400000000000000
#define PHY_CTRL_1_RD_ODT_SHIFT				24
#define PHY_CTRL_REG_1_MASK					0xffffffff

#define CONF_CTL_118_REG					0x760
#define ADDRESS_MIRROR_MASK					0xfffffff0ffffffff
#define ADDRESS_MIRROR_OFFSET				32

#define CONF_CTL_150_REG					0x960

#define CONF_CTL_151_REG					0x970
#define INT_ACK_OFFSET						24
#define INT_ACK_MASK						0xfffffe0000ffffff
#define INT_CLEAR_VALUE						0x1ffff

#define CONF_CTL_158_REG					0x9e0
#define MR2_DATA_0_OFFSET					48
#define MR2_DATA_0_MASK						0x8000ffffffffffff

#define CONF_CTL_159_REG					0x9f0
#define MR2_DATA_3_OFFSET					32
#define MR2_DATA_3_MASK						0xffff8000ffffffff
#define MR2_DATA_2_OFFSET					16
#define MR2_DATA_2_MASK						0xffffffff8000ffff
#define MR2_DATA_1_OFFSET					0
#define MR2_DATA_1_MASK						0xffffffffffff8000
#define MR2_ODT_WR_OFFSET					9

#define CONF_CTL_163_REG					0xa30
#define RDLVL_DELAY_2_OFFSET				48
#define RDLVL_DELAY_2_MASK					0x0000ffffffffffff
#define RDLVL_DELAY_1_OFFSET				32
#define RDLVL_DELAY_1_MASK					0xffff0000ffffffff
#define RDLVL_DELAY_0_OFFSET				16
#define RDLVL_DELAY_0_MASK					0xffffffff0000ffff

#define CONF_CTL_164_REG					0xa40
#define RDLVL_DELAY_6_OFFSET				48
#define RDLVL_DELAY_6_MASK					0x0000ffffffffffff
#define RDLVL_DELAY_5_OFFSET				32
#define RDLVL_DELAY_5_MASK					0xffff0000ffffffff
#define RDLVL_DELAY_4_OFFSET				16
#define RDLVL_DELAY_4_MASK					0xffffffff0000ffff
#define RDLVL_DELAY_3_OFFSET				0
#define RDLVL_DELAY_3_MASK					0xffffffffffff0000

#define CONF_CTL_165_REG					0xa50
#define RDLVL_DELAY_8_OFFSET				16
#define RDLVL_DELAY_8_MASK					0xffffffff0000ffff
#define RDLVL_DELAY_7_OFFSET				0
#define RDLVL_DELAY_7_MASK					0xffffffffffff0000

#define CONF_CTL_167_REG					0xa70
#define RDLVL_GATE_DELAY_0_OFFSET			48
#define RDLVL_GATE_DELAY_0_MASK				0x0000ffffffffffff

#define CONF_CTL_168_REG					0xa80
#define RDLVL_GATE_DELAY_4_OFFSET			48
#define RDLVL_GATE_DELAY_4_MASK				0x0000ffffffffffff
#define RDLVL_GATE_DELAY_3_OFFSET			32
#define RDLVL_GATE_DELAY_3_MASK				0xffff0000ffffffff
#define RDLVL_GATE_DELAY_2_OFFSET			16
#define RDLVL_GATE_DELAY_2_MASK				0xffffffff0000ffff
#define RDLVL_GATE_DELAY_1_OFFSET			0
#define RDLVL_GATE_DELAY_1_MASK				0xffffffffffff0000

#define CONF_CTL_169_REG					0xa90
#define RDLVL_GATE_DELAY_8_OFFSET			48
#define RDLVL_GATE_DELAY_8_MASK				0x0000ffffffffffff
#define RDLVL_GATE_DELAY_7_OFFSET			32
#define RDLVL_GATE_DELAY_7_MASK				0xffff0000ffffffff
#define RDLVL_GATE_DELAY_6_OFFSET			16
#define RDLVL_GATE_DELAY_6_MASK				0xffffffff0000ffff
#define RDLVL_GATE_DELAY_5_OFFSET			0
#define RDLVL_GATE_DELAY_5_MASK				0xffffffffffff0000

#define CONF_CTL_175_REG					0xaf0
#define WRLVL_DELAY_1_OFFSET				48
#define WRLVL_DELAY_1_MASK					0x0000ffffffffffff
#define WRLVL_DELAY_0_OFFSET				32
#define WRLVL_DELAY_0_MASK					0xffff0000ffffffff

#define CONF_CTL_176_REG					0xb00
#define WRLVL_DELAY_5_OFFSET				48
#define WRLVL_DELAY_5_MASK					0x0000ffffffffffff
#define WRLVL_DELAY_4_OFFSET				32
#define WRLVL_DELAY_4_MASK					0xffff0000ffffffff
#define WRLVL_DELAY_3_OFFSET				16
#define WRLVL_DELAY_3_MASK					0xffffffff0000ffff
#define WRLVL_DELAY_2_OFFSET				0
#define WRLVL_DELAY_2_MASK					0xffffffffffff0000

#define CONF_CTL_177_REG					0xb10
#define WRLVL_DELAY_8_OFFSET				32
#define WRLVL_DELAY_8_MASK					0xffff0000ffffffff
#define WRLVL_DELAY_7_OFFSET				16
#define WRLVL_DELAY_7_MASK					0xffffffff0000ffff
#define WRLVL_DELAY_6_OFFSET				0
#define WRLVL_DELAY_6_MASK					0xffffffffffff0000

#endif	//__DDR_PARAM_DEFINE__

