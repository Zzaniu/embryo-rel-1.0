
/**
 * Copyright(c) 2015-7-10 Shangwen Wu	
 *
 * LS2H DDR3控制器函数集（仅考虑MC0） 
 * 
 */

/*
 * 描述：ddr_config配置内存的所有寄存器参数
 * 参数：s3标识是否需要level，t7表示DDR当前节点配置寄存器首地址
 * 返回：
 * 寄存器使用：t8（返回地址）t7（MC配置基址）
 */
	.global	ddr_config
	.ent	ddr_config
ddr_config:
	move		t8, ra
	bal			enable_ddr_config_space		//ddr_param_modify_func.S
	nop
	beq			s3, zero, 1f
	nop
	MC0_GET_INFO_TO_a1(SDRAM_TYPE)			//need level
	dli			t0, 0x02
	beq			a1, t0, 2f
	nop
	dli			t0, 0x03
	beq			a1, t0, 3f
	nop
	b 			ddr_config_end
	nop

2:											//ddr2
	MC0_GET_INFO_TO_a1(DIMM_TYPE)		
	dli			t0, 0x00
	beq			a1, t0, 21f
	nop
	dli			t0, 0x01
	beq			a1, t0, 22f
	nop
21:											//unbuffered DIMM	
	dla			a0, ddr2_UDIMM_reg_data_mc0 
	b			4f
	nop
22:											//register DIMM
	dla			a0, ddr2_RDIMM_reg_data_mc0
	b			4f
	nop

3:											//ddr3		
	MC0_GET_INFO_TO_a1(DIMM_TYPE)		
	dli			t0, 0x00
	beq			a1, t0, 31f
	nop
	dli			t0, 0x01
	beq			a1, t0, 32f
	nop
31:											//unbuffered DIMM	
	dla			a0, ddr3_UDIMM_reg_data_mc0
	b			4f
	nop
32:											//register DIMM
	dla			a0, ddr3_RDIMM_reg_data_mc0
	b			4f
	nop

1:											//leveled
	dla			a0, ddr_reg_data_mc0_leveled

4:											//读取指定地址（指令地址）的值，并将该值写入到DDR配置寄存器
	daddu		a0, s0
	move		a1, t7	
	dli			a2, LS2H_DDR_PARAM_NUM-1
41:
	ld			t0, 0x00(a0)
	sd			t0, 0x00(a1)
	daddu		a0, 0x08
	daddu 		a1, 0x10
	bgtz		a2, 41b
	daddu		a2, -1

	beq			s3, zero, 88f 				/* 已经校准的参数无需进行参数重写 */
	nop

	MC0_GET_INFO_TO_a1(SDRAM_WIDTH)			/* 下面针对*16位宽模式（默认）以外的配置进行tFAW时序修正 */
	bnez		a1, 5f
	nop
	MC0_GET_INFO_TO_a1(SDRAM_TYPE)
	dli			t0, 0x02
	beq			a1, t0, 2f
	nop
	dli			a0, 0x0c + 2				//DDR3
	beq			a1, t0, 3f
	nop		
2:
	dli			a0, 0x0f + 2				//DDR2
3:
	ld			t0, CONF_CTL_10_REG(t7)
	and			t0, TFAW_MASK
	dsll		a0, TFAW_OFFSET
	or			t0, a0
	sd			t0, CONF_CTL_10_REG(t7)

5:											/* 根据每块颗粒容量，对tRFC（刷新一次需要的时钟周期数）和TXSNR时序参数进行修正 */	
	move		a3, zero					/* 计算DIMM数 */	
	MC0_GET_INFO_TO_a1(MC_CS_MAP)
	and			a2, a1, 0x01	
	daddu		a3, a2
	dsrl		a1, 1
	and			a2, a1, 0x01	
	daddu		a3, a2
	dsrl		a1, 1
	and			a2, a1, 0x01	
	daddu		a3, a2
	dsrl		a1, 1
	and			a2, a1, 0x01	
	daddu		a3, a2
	
	MC0_GET_INFO_TO_a1(MEMORY_SIZE)			/* 计算每个DIMM容量 */
	divu		a0, a1, a3
	dsll		a0, 3						//byte-->bit
	
	MC0_GET_INFO_TO_a1(SDRAM_WIDTH)			/* 计算每个DIMM的颗粒数 */
	move		a2, a1
	MC0_GET_INFO_TO_a1(DIMM_WIDTH)	
	beqz		a1,	64f
	nop
32:											//32bit DIMM		
	bnez		a2, 16f	
	nop
8:											//8bit SDRAM
	dli			a3, 2
	b			51f
	nop
16:
	dli			a3, 1						//16bit SDRAM
	b			51f
	nop
64:											//64bit DIMM
	bnez		a2, 16f	
	nop
8:											//8bit SDRAM
	dli			a3, 3
	b			51f
	nop
16:
	dli			a3, 2						//16bit SDRAM
	b			51f
	nop

51:
	dsrl		a0, a3						//dimm容量（bit）/ 颗粒数

	MC0_GET_INFO_TO_a1(SDRAM_TYPE)			
	dli			a2, 0x03
	beq			a1, a2, 1f
	nop

	dli			a1, 0x01
	beq			a0, a1, DDR2_density_512Mb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR2_density_1Gb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR2_density_2Gb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR2_density_4Gb
	nop
	b			6f							//默认值，无需修改
	nop

DDR2_density_512Mb:
	dli			a1, 0x2a + TRFC_MARGIN
	b			52f
	nop
DDR2_density_1Gb:
	dli			a1, 0x33 + TRFC_MARGIN
	b			52f
	nop
DDR2_density_2Gb:
	dli			a1, 0x4e + TRFC_MARGIN
	b			52f
	nop
DDR2_density_4Gb:
	dli			a1, 0x83 + TRFC_MARGIN
	b			52f
	nop

1:
	dli			a1, 0x01
	beq			a0, a1, DDR3_density_512Mb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR3_density_1Gb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR3_density_2Gb
	nop
	dsll		a1, 1
	beq			a0, a1, DDR3_density_4Gb
	nop
	b			6f							//默认值，无需修改
	nop

DDR3_density_512Mb:
	dli			a1, 0x24 + TRFC_MARGIN
	b			52f
	nop
DDR3_density_1Gb:
	dli			a1, 0x2c + TRFC_MARGIN
	b			52f
	nop
DDR3_density_2Gb:
	dli			a1, 0x40 + TRFC_MARGIN
	b			52f
	nop
DDR3_density_4Gb:
	dli			a1, 0x78 + TRFC_MARGIN
	b			52f
	nop

52:
	ld			t0, CONF_CTL_12_REG(t7)
	and			t0, TRFC_MASK
	dsll		a2, a1, TRFC_OFFSET
	or   		t0, a2
	sd			t0, CONF_CTL_12_REG(t7)
		
	ld			t0, CONF_CTL_20_REG(t7)
	and			t0, TXSNR_MASK
	daddu		a2, a1, 0x04
	dsll		a2, TXSNR_OFFSET
	or   		t0, a2	
	sd			t0, CONF_CTL_20_REG(t7)

6:
	MC0_GET_INFO_TO_a1(SDRAM_EIGHT_BANK)	/* rewrite 8 bank */	
	ld			t0, CONF_CTL_01_REG(t7)
	and			t0, EIGHT_BANK_MODE_MASK
	dsll		a1, EIGHT_BANK_MODE_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_01_REG(t7)
	
	ld			t0, CONF_CTL_05_REG(t7)		/* rewrite row and col pins */
	and			t0, ROWCOL_SIZE_MASK
	MC0_GET_INFO_TO_a1(SDRAM_ADDR_PINS)			
	dsll		a1, ADDR_PINS_OFFSET
	or   		t0, a1	
	MC0_GET_INFO_TO_a1(SDRAM_COL_SIZE)			
	daddu		a1, 2						/* bad codes */
	dsll		a1, COLNUM_SIZE_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_05_REG(t7)
		
	MC0_GET_INFO_TO_a1(MC_CS_MAP)			/* rewrite cs map */	
	ld			t0, CONF_CTL_07_REG(t7)
	and			t0, CS_MAP_MASK
	dsll		a1, CS_MAP_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_07_REG(t7)
	
	MC0_GET_INFO_TO_a1(ADDR_MIRROR)			/* rewrite address mirror */	
	ld			t0, CONF_CTL_118_REG(t7)
	and			t0, ADDRESS_MIRROR_MASK
	beqz		a1, 1f
	nop
	dli			a1, 0x0a
1:
	dsll		a1, ADDRESS_MIRROR_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_118_REG(t7)
	
	/* 以下代码我也弄不清在搞啥，只知道选中某个片选信号将交叉的打开另外一个（多个）ODT电阻 */
	dli			a2, 0x0804020100000000
	MC0_GET_INFO_TO_a1(MC_CS_MAP)			/* rewrite the ODT-cs map(defualt 0x0804020100000000) */
	
	/* 检查cs[3]是否为1，是则将cs3和cs2分别打开对方动态ODT */
	dsrl		a0, a1, 3
	and			a0, 0x01
	beqz		a0, 1f
	nop
	and			a2, 0x0000ffffffffffff
	or			a2, 0x0408000000000000
1:
	/* 检查cs[1]是否为1，是则将cs1和cs0分别打开对方动态ODT */
	dsrl		a0, a1, 1
	and			a0, 0x01
	beqz		a0, 1f
	nop
	and			a2, 0xffff0000ffffffff
	or			a2, 0x0000010200000000
1:
	/* 检查当前系统是否有2个DIMM */
	dsrl		a0, a1, 2
	xor			a0, a0, a1				
	and			a0, 0x01					/* cs[2]、cs[0]相等 */
	bnez		a0, 1f						
	nop
	or			a2, 0x0101040401010404		/* 有两个DIMM, cs[2] = cs[1] = 1 */
	/* 检查cs[1]是否为1，是则将cs1和cs0分别打开对方动态ODT */
	dsrl		a0, a1, 1
	and			a0, 0x01
	beqz		a0, 2f
	nop
	or			a2, 0x0202000002020000
	b			2f
	nop
		
1:											/* 只有一个DIMM，cs[0] != 0 */	
	MC0_GET_INFO_TO_a1(SDRAM_TYPE)	
	dli			t0, 0x02
	beq			a1, t0, 2f
	nop

	/* 对于DDR3需要禁止动态ODT，即disable模式寄存器2的wr_rtt */
	dli			a0, 0x03	
	ld			t0, CONF_CTL_158_REG(t7)						//cs0
	dsll		a1, a0, MR2_DATA_0_OFFSET + MR2_ODT_WR_OFFSET
	not			a1, a1
	and			t0, a1
	sd			t0, CONF_CTL_158_REG(t7)
	ld			t0, CONF_CTL_159_REG(t7)	
	dsll		a1, a0, MR2_DATA_1_OFFSET + MR2_ODT_WR_OFFSET	//cs1
	not			a1, a1
	and			t0, a1
	dsll		a1, a0, MR2_DATA_2_OFFSET + MR2_ODT_WR_OFFSET	//cs2
	not			a1, a1
	and			t0, a1
	dsll		a1, a0, MR2_DATA_3_OFFSET + MR2_ODT_WR_OFFSET	//cs3
	not			a1, a1
	and			t0, a1
	sd			t0, CONF_CTL_159_REG(t7)

2:
	sd			a2, CONF_CTL_08_REG(t7)		/* 保存上述设置的值 */	
	
	MC0_GET_INFO_TO_a1(DIMM_WIDTH)			/* rewrite reduc */	
	ld			t0, CONF_CTL_03_REG(t7)
	and			t0, REDUC_MASK
	dsll		a1, REDUC_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_03_REG(t7)
	
	/* 禁止ECC，用以进行后续的参数校准，后面将使能 */
	ld			t0, CONF_CTL_04_REG(t7)
	and			t0, CTRL_RAW_MASK
	dli			a0, 0x02					//not use ecc device
	dsll		a0, CTRL_RAW_OFFSET
	or			t0, a0
	sd			t0, CONF_CTL_04_REG(t7)

88:
	sync
	ld			t0, CONF_CTL_03_REG(t7)
	or			t0, START_DDR
	sd			t0, CONF_CTL_03_REG(t7)
	sync

	//wait a while
	dli			t0, 0x100
1:	
	bnez		t0, 1b
	daddu		t0, -1

1:
	ld			t0, CONF_CTL_150_REG(t7)
	and			t0, 0x100
	beqz		t0, 1b
	nop

1:
	ld			t0, CONF_CTL_01_REG(t7)
	and			t0, DLL_LOCKED
	beqz		t0, 1b
	nop

ddr_config_end:	
	bal			disable_ddr_config_space
	nop
	jr			t8
	nop
	.end	ddr_config

/*
 * 描述：MC初始化
 * 参数：v3（是否需要level） 
 * 返回：
 * 寄存器使用：t9（返回地址）t7（DDR控制器配置基址）
 */
	.global	mc_init
	.ent	mc_init
mc_init:
	move		t9, ra
	GET_CUR_NODE_CONFIG_REG_TO_t7 
	bal			ddr_config		
	nop										//此时DDR已经启动		
	
	beqz		s3, 88f
	nop	
	
	/* 下面将进行DDR参数校准 */
	bal			ddr_auto_level				//开始自动DDR参数校准
	nop

88:
	/* 清除初始化过程中产生的中断状态 */
	bal			enable_ddr_config_space
	nop
	
	dli			a1, INT_CLEAR_VALUE
	ld			t0, CONF_CTL_151_REG(t7)
	and			t0, INT_ACK_MASK
	dsll		a1, INT_ACK_OFFSET
	or   		t0, a1	
	sd			t0, CONF_CTL_151_REG(t7)

	dli			a0, 0x400					//wait a while
1:	
	beqz		a0, 1b
	daddu		a0, -1

	dli			a1, INT_CLEAR_VALUE
	ld			t0, CONF_CTL_151_REG(t7)
	and			t0, INT_ACK_MASK
	dsll		a1, INT_ACK_OFFSET
	not			a1, a1
	and   		t0, a1	
	sd			t0, CONF_CTL_151_REG(t7)

	bal			disable_ddr_config_space
	nop

	jr			t9
	nop
	.end	mc_init

/*
 * 描述：检查当前BIOS上电过程是否需要对DDR进行参数校准
 * 参数： 
 * 返回：v0，为1表示需要进行参数校准
 * 寄存器使用：a0, a1, v0
 */
LEAF(check_need_level)
	dla			a0, mc0_level_info
	daddu		a0, a0, s0
	ld			a1, 0x00(a0)		
	and			a1, a1, 0x01

	beqz		a1, 1f
	move		v0, zero
	b			2f
	nop	
1:
	dli			v0, 0x1	
2:
	jr			ra
	nop
END(check_need_level)

/*************************** 包含其他依赖汇编文件 ***************************/
/* DDR函数集 */
#include "ddr_auto_level.S"				/* 包含DDR配置函数 */

