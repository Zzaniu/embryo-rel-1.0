
/**
 * Copyright(c) 2018-4-30 Shangwen Wu	
 *
 * MIPS架构相关的cache处理函数
 * 
 */
#include <asm/regdef.h>
#include <asm/cpu.h>
#include <asm/cache.h>
#include <asm/asm.h>

/* 
 * void cpu_cache_conf(void)
 * 描述： 根据CPU类型配置几个cache相关的全局变量
 * 使用寄存器：t0(cache类型) t1(icache大小) t2(dcache大小) t3(icache行长) 
 *			   t4(dcache行长) t5(二级cache大小) t6(三级cache大小) 
 *			   v0(CPU类型) v1(临时变量)
 */
LEAF(cpu_cache_conf)
	.set			noreorder

	mfc0			v0, CP0_PRID	
	andi			v0, MIPS_CPU_MASK

	li				v1, (MIPS_R4600<<8)
	beq				v0, v1, config_r4600
	nop
	li				v1, (MIPS_R4700<<8)
	beq				v0, v1, config_r4700
	nop
	li				v1, (MIPS_GODSON1<<8)
	beq				v0, v1, config_godson1
	nop
	li				v1, (MIPS_GODSON2<<8)
	beq				v0, v1, config_godson2
	nop

config_r4600:
	/* 添加CPU cache信息 */
	b				config_result
	nop

config_r4700:
	/* 添加CPU cache信息 */
	b				config_result
	nop

config_godson1:			//64KB dcahe 64KB icache no l2-cache
	li				t0, CACHE_TYPE_4WAY
	li				t1,	64*1024 
	li				t2,	64*1024 
	li				t3,	32 
	li				t4,	32 
	move			t5,	zero
	move			t6,	zero
	b				config_result
	nop

config_godson2:			//64KB dcache 64KB icache 512KB chip on l2-cache
	li				t0, CACHE_TYPE_4WAY
	ori				t0, CACHE_TYPE_HAS_L2
	li				t1,	64*1024 
	li				t2,	64*1024 
	li				t3,	32 
	li				t4,	32 
	li				t5,	512*1024 
	move			t6,	zero
	b				config_result
	nop

/* 写入结果 */
config_result:
	sw				t0, cpu_cache_type
	sw				t1,	cpu_primary_icache_szie 
	sw				t2,	cpu_primary_dcache_szie 
	sw				t3,	cpu_primary_icache_line_szie 
	sw				t4,	cpu_primary_dcache_line_szie 
	sw				t5,	cpu_secondry_cache_szie
	sw				t6,	cpu_tertiary_cache_szie
	andi			v1, t0, CACHE_TYPE_WAY_MASK
	srl				t1, v1
	sw				t1, cpu_primary_icache_set_szie
	srl				t2, v1
	sw				t1, cpu_primary_dcache_set_szie

	jr				ra
	nop
	.set			reorder
END(cpu_cache_conf)


.set	push
.set	mips3	

/**
 * void cpu_flush_cache(void)
 * 描述：索引型flush所有缓存
 * 寄存器使用t0-t4, v0
 */
LEAF(cpu_flush_cache)
	.set		noreorder

	mfc0			t0, CP0_PRID 
	li				t1, 0x6305			#godson2g prid
	beq				t0, t1, for_godson2g		
	li				t0, 0x6303			#godson2f prid
	beq				t0, t1, for_godson2f
	li				t0, 0x6302			#godson2e prid
	bne				t0, t1, non_godson2e
	nop
	
/* 对于龙芯2E/2F/2G仅需操作二级cache？？ */
for_godson2e:
for_godson2g:
for_godson2f:

	lw				t4, cpu_secondry_cache_szie
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 32
1:
	cache			Index_WriteBack_Inv_S, 0(t1)
	cache			Index_WriteBack_Inv_S, 1(t1)
	cache			Index_WriteBack_Inv_S, 2(t1)
	cache			Index_WriteBack_Inv_S, 3(t1)
	
	blt				t1, t2, 1b
	addu			t1, 32
	
	b				88f
	nop

non_godson2e:
	/* flush icache */
	lw				t4, cpu_primary_icache_szie
	lw				t3, cpu_primary_icache_line_szie
	
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 128

	subu			t3, 16					//cache line == 16？
1:
	bnez			t3, 2f
	cache			Index_Invalidate_I, 0(t1)	//注意：icache仅有作废操作，没有写回操作

	cache			Index_Invalidate_I, 16(t1)
	cache			Index_Invalidate_I, 48(t1)
	cache			Index_Invalidate_I, 80(t1)
	cache			Index_Invalidate_I, 112(t1)

2:
	cache			Index_Invalidate_I, 32(t1)
	cache			Index_Invalidate_I, 64(t1)
	cache			Index_Invalidate_I, 96(t1)
	
	blt				t1, t2, 1b
	addu			t1, 128
	
	/* flush dcache */	
	lw				t4, cpu_primary_dcache_szie
	lw				t3, cpu_primary_dcache_line_szie
	
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 128

	subu			t3, 16					//cache line == 16？
10:
	bnez			t3, 13f
	cache			Index_WriteBack_Inv_D, 0(t1)

	cache			Index_WriteBack_Inv_D, 16(t1)
	cache			Index_WriteBack_Inv_D, 48(t1)
	cache			Index_WriteBack_Inv_D, 80(t1)
	cache			Index_WriteBack_Inv_D, 112(t1)

	/* 针对龙芯CPU的奇葩操作 */
	andi			t0, MIPS_CPU_MASK
	li				v0, (MIPS_GODSON1<<8)
	beq				t0, v0, 11f
	li				v0, (MIPS_GODSON2<<8)
	bne				t0, v0, non_godson_1
	nop

11:
	cache			Index_WriteBack_Inv_D, 17(t1)
	cache			Index_WriteBack_Inv_D, 49(t1)
	cache			Index_WriteBack_Inv_D, 81(t1)
	cache			Index_WriteBack_Inv_D, 113(t1)

	cache			Index_WriteBack_Inv_D, 18(t1)
	cache			Index_WriteBack_Inv_D, 50(t1)
	cache			Index_WriteBack_Inv_D, 82(t1)
	cache			Index_WriteBack_Inv_D, 114(t1)

	cache			Index_WriteBack_Inv_D, 19(t1)
	cache			Index_WriteBack_Inv_D, 51(t1)
	cache			Index_WriteBack_Inv_D, 83(t1)
	cache			Index_WriteBack_Inv_D, 115(t1)

non_godson_1:

13:
	cache			Index_WriteBack_Inv_D, 32(t1)
	cache			Index_WriteBack_Inv_D, 64(t1)
	cache			Index_WriteBack_Inv_D, 96(t1)
	
	/* 针对龙芯CPU的奇葩操作 */
	andi			t0, MIPS_CPU_MASK
	li				v0, (MIPS_GODSON1<<8)
	beq				t0, v0, 14f
	li				v0, (MIPS_GODSON2<<8)
	bne				t0, v0, non_godson_2
	nop

14:
	cache			Index_WriteBack_Inv_D, 1(t1)
	cache			Index_WriteBack_Inv_D, 33(t1)
	cache			Index_WriteBack_Inv_D, 65(t1)
	cache			Index_WriteBack_Inv_D, 97(t1)
	
	cache			Index_WriteBack_Inv_D, 2(t1)
	cache			Index_WriteBack_Inv_D, 34(t1)
	cache			Index_WriteBack_Inv_D, 66(t1)
	cache			Index_WriteBack_Inv_D, 98(t1)
	
	cache			Index_WriteBack_Inv_D, 3(t1)
	cache			Index_WriteBack_Inv_D, 35(t1)
	cache			Index_WriteBack_Inv_D, 67(t1)
	cache			Index_WriteBack_Inv_D, 99(t1)

non_godson_2:
	blt				t1, t2, 10b
	addu			t1, 128
	
	/* flush chip on l2 cache */
	lw				t0, cpu_cache_type
	nop
	andi			v0, t0, CACHE_TYPE_HAS_L2
	beqz			v0, 30f
	nop

	lw				t4, cpu_secondry_cache_szie
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 32
20:
	cache			Index_WriteBack_Inv_S, 0(t1)
	cache			Index_WriteBack_Inv_S, 1(t1)
	cache			Index_WriteBack_Inv_S, 2(t1)
	cache			Index_WriteBack_Inv_S, 3(t1)
	
	blt				t1, t2, 20b
	addu			t1, 32
	
	/* flush chip off l2 cache */
30:
	andi			v0, t0, CACHE_TYPE_HAS_XL2
	beqz			v0, 40f
	nop
	
	mtc0			zero, CP0_TAGLO
	lw				t4, cpu_secondry_cache_szie
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 4096 
31:
	cache			Invalidate_Page_S, 0(t1)
	
	blt				t1, t2, 31b
	addu			t1, 4096

	/* flush l3 cache */
40:
	andi			v0, t0, CACHE_TYPE_HAS_L3 
	beqz			v0, 88f
	nop
	
	mtc0			zero, CP0_TAGLO
	lw				t4, cpu_tertiary_cache_szie
	li				t1, CPU_KSEG_CACHED		//使用kseg0地址，防止引起TLB重填异常
	addu			t2, t4, t1				//bad code，当t4足够大以至于进行64位的符号扩展时，可能出现出现问题
	subu			t2, 4096 
41:
	cache			Invalidate_Page_T, 0(t1)
	
	blt				t1, t2, 41b
	addu			t1, 4096

88:
	jr				ra
	nop
	.set			reorder
END(cpu_flush_cache)

/**
 * void cpu_flush_icache(vm_offset_t addr, vm_size_t len)
 * 描述：索引型flush指定区域内的指令缓存
 * 注意：该函数将对行长作出假设，单路cache的行长将被认为是16字节，而多路行长统一认为是32字节
 * 寄存器使用t0-t3, a0-a3
 */
LEAF(cpu_flush_icache)
	.set		noreorder

	lw				a2, cpu_primary_icache_set_szie
	li				t1, CPU_KSEG_CACHED
	subu			a3, a2, 1
	and				a0, a3			//计算索引
	andi			a3, a0, 127		//调整起始地址t1为向下128字节对齐
	subu			a0, a3
	addu			t1, a0			//操作基地址
	addu			t2, a1, a3		//计算循环次数t2，每次操作128字节
	addu			t2, 127			//t2向上128字节对齐
	blt				t2, a2, 1f
	srl				t2, t2, 7

	srl				t2, a2, 7
1:
	lw				t0, cpu_cache_type 
	nop
	andi			t0, CACHE_TYPE_WAY_MASK
2:
	subu			t2, 1
	subu			a3, t0, 1			//dir: <0; 2way: =0; 4way: >0
	bgez			a3, 3f
	cache			Index_Invalidate_I, 0(t1)

	/* 单路，这里假设单路cache行长为16字节 */
	cache			Index_Invalidate_I, 16(t1)
	cache			Index_Invalidate_I, 48(t1)
	cache			Index_Invalidate_I, 80(t1)
	
	b				4f
	cache			Index_Invalidate_I, 112(t1)
3:
	/* 双路 */
	addu			t3, t1, a2		//操作set B
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	subu			a3, 1			//2way: <0; 4way: =0; 8way: >0
	bltz			a3, 4f
	addu			t3, t1, a2		//操作set C

	/* 4路 */
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	addu			t3, t1, a2		//操作set D
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	beqz			a3, 4f
	addu			t3, t1, a2		//操作set E
		
	/* 8路 */
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	addu			t3, t1, a2		//操作set F
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	addu			t3, t1, a2		//操作set G
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)
	
	addu			t3, t1, a2		//操作set H
	cache			Index_Invalidate_I, 0(t3)
	cache			Index_Invalidate_I, 32(t3)
	cache			Index_Invalidate_I, 64(t3)
	cache			Index_Invalidate_I, 96(t3)

4:									//操作set A
	cache			Index_Invalidate_I, 32(t1)
	cache			Index_Invalidate_I, 64(t1)
	cache			Index_Invalidate_I, 96(t1)

	bnez			t2, 2b
	addu			t1, 128

	jr				ra
	nop
	.set			reorder
END(cpu_flush_icache)

/**
 * void cpu_flush_dcache(vm_offset_t addr, vm_size_t len)
 * 描述：索引型flush指定区域内的数据缓存
 * 注意：该函数将对行长作出假设，单路cache的行长将被认为是16字节，而多路行长统一认为是32字节
 * 寄存器使用t0-t5, a0-a3, v0-v1
 */
LEAF(cpu_flush_dcache)
	.set		noreorder

	lw				a2, cpu_primary_dcache_set_szie
	li				t1, CPU_KSEG_CACHED
	subu			a3, a2, 1
	and				a0, a3			//计算索引
	andi			a3, a0, 127		//调整起始地址t1为向下128字节对齐
	subu			a0, a3
	addu			t1, a0			//操作基地址
	addu			t2, a1, a3		//计算循环次数t2，每次操作128字节
	addu			t2, 127			//t2向上128字节对齐
	blt				t2, a2, 1f
	srl				t2, t2, 7

	srl				t2, a2, 7
1:
	/* t0：cache路数，v0：cpu prid */
	lw				t0, cpu_cache_type 
	mfc0			v0, CP0_PRID 
	andi			t0, CACHE_TYPE_WAY_MASK
	andi			v0, MIPS_CPU_MASK

2:
	subu			t2, 1
	bnez			t0, 3f			//是否单路
	li				t4, 0x1			//t4: 当前路数，t5：最后一路

	/* 针对龙芯CPU的奇葩操作 */
	li				v1, (MIPS_GODSON1<<8)
	beq				v0, v1, 21f
	li				v1, (MIPS_GODSON2<<8)
	bne				v0, v1, non_godson_3
	cache			Index_WriteBack_Inv_D, 16(t1)
21:
	cache			Index_WriteBack_Inv_D, 17(t1)
	cache			Index_WriteBack_Inv_D, 49(t1)
	cache			Index_WriteBack_Inv_D, 81(t1)
	cache			Index_WriteBack_Inv_D, 113(t1)
		
	cache			Index_WriteBack_Inv_D, 18(t1)
	cache			Index_WriteBack_Inv_D, 50(t1)
	cache			Index_WriteBack_Inv_D, 82(t1)
	cache			Index_WriteBack_Inv_D, 114(t1)
		
	cache			Index_WriteBack_Inv_D, 19(t1)
	cache			Index_WriteBack_Inv_D, 51(t1)
	cache			Index_WriteBack_Inv_D, 83(t1)
	cache			Index_WriteBack_Inv_D, 115(t1)
		
non_godson_3:
	cache			Index_WriteBack_Inv_D, 48(t1)
	cache			Index_WriteBack_Inv_D, 80(t1)
	cache			Index_WriteBack_Inv_D, 112(t1)

3:
	sll				t5, t4, t0
	move			t3, t1
	
	/* 循环操作每一路cache set */
4:
	/* 针对龙芯CPU的奇葩操作 */
	li				v1, (MIPS_GODSON1<<8)
	beq				v0, v1, 41f
	li				v1, (MIPS_GODSON2<<8)
	bne				v0, v1, non_godson_4
	cache			Index_WriteBack_Inv_D, 0(t3)
	
41:
	cache			Index_WriteBack_Inv_D, 1(t3)
	cache			Index_WriteBack_Inv_D, 33(t3)
	cache			Index_WriteBack_Inv_D, 65(t3)
	cache			Index_WriteBack_Inv_D, 97(t3)

	cache			Index_WriteBack_Inv_D, 2(t3)
	cache			Index_WriteBack_Inv_D, 34(t3)
	cache			Index_WriteBack_Inv_D, 66(t3)
	cache			Index_WriteBack_Inv_D, 98(t3)

	cache			Index_WriteBack_Inv_D, 3(t3)
	cache			Index_WriteBack_Inv_D, 35(t3)
	cache			Index_WriteBack_Inv_D, 67(t3)
	cache			Index_WriteBack_Inv_D, 99(t3)

non_godson_4:
	cache			Index_WriteBack_Inv_D, 32(t3)
	cache			Index_WriteBack_Inv_D, 64(t3)
	cache			Index_WriteBack_Inv_D, 96(t3)
	
	addu			t3, a2
	blt				t4, t5, 4b
	addu			t4, 1
	
	bnez			t2, 2b
	addu			t1, 128

	jr				ra
	nop
	.set			reorder
END(cpu_flush_dcache)

/**
 * void cpu_hit_flush_dcache(vm_offset_t addr, vm_size_t len)
 * 描述：命中型flush指定区域内的数据缓存
 * 注意：该函数将对行长作出假设，单路cache的行长将被认为是16字节，而多路行长统一认为是32字节
 * 		 传入的地址参数，要确保为kseg0或者经过tlb映射的虚拟地址，防止发生TLB重填异常
 * 寄存器使用a0-a1, v0
 */
LEAF(cpu_hit_flush_dcache)
	.set			noreorder

	beqz			a1, 3f
	addu			a1, a0, a1
	lw				v0, cpu_cache_type
	addu			a1, 127				//大小向上128字节对齐
	and				a0, -128			//起始地址向下128字节对齐
	subu			a1, a0
	srl				a1, 7
	
	andi			v0, CACHE_TYPE_WAY_MASK

1:
	beqz			v0, 2f				//单路？
	subu			a1, 1
	
	cache 			Hit_WriteBack_Inv_D, 16(a0)
	cache 			Hit_WriteBack_Inv_D, 48(a0)
	cache 			Hit_WriteBack_Inv_D, 80(a0)
	cache 			Hit_WriteBack_Inv_D, 112(a0)
	
2:
	cache 			Hit_WriteBack_Inv_D, 0(a0)
	cache 			Hit_WriteBack_Inv_D, 32(a0)
	cache 			Hit_WriteBack_Inv_D, 64(a0)
	cache 			Hit_WriteBack_Inv_D, 96(a0)

	bnez			a1, 1b
	addu			a0, 128

3:
	jr				ra
	nop
	.set			reorder
END(cpu_hit_flush_dcache)

/**
 * void cpu_hit_flush_scache(vm_offset_t addr, vm_size_t len)
 * 描述：命中型flush指定区域内的二级缓存
 * 注意：该函数将对行长作出假设，路行长统一认为是32字节
 * 		 传入的地址参数，要确保为kseg0或者经过tlb映射的虚拟地址，防止发生TLB重填异常
 * 寄存器使用a0-a1
 */
LEAF(cpu_hit_flush_scache)
	.set			noreorder

	beqz			a1, 2f
	addu			a1, a0, a1
	addu			a1, 127				//大小向上128字节对齐
	and				a0, -128			//起始地址向下128字节对齐
	subu			a1, a0
	srl				a1, 7
	
1:
	subu			a1, 1
	
	cache 			Hit_WriteBack_Inv_S, 0(a0)
	cache 			Hit_WriteBack_Inv_S, 32(a0)
	cache 			Hit_WriteBack_Inv_S, 64(a0)
	cache 			Hit_WriteBack_Inv_S, 96(a0)

	bnez			a1, 1b
	addu			a0, 128

2:
	jr				ra
	nop
	.set			reorder
END(cpu_hit_flush_scache)

/**
 * void cpu_hit_invalidate_dcache(vm_offset_t addr, vm_size_t len)
 * 描述：命中型作废指定区域内的数据缓存
 * 注意：该函数将对行长作出假设，单路cache的行长将被认为是16字节，而多路行长统一认为是32字节
 * 		 传入的地址参数，要确保为kseg0或者经过tlb映射的虚拟地址，防止发生TLB重填异常
 * 寄存器使用a0-a1, v0
 */
LEAF(cpu_hit_invalidate_dcache)
	.set			noreorder

	beqz			a1, 3f
	addu			a1, a0, a1
	lw				v0, cpu_cache_type
	addu			a1, 127				//大小向上128字节对齐
	and				a0, -128			//起始地址向下128字节对齐
	subu			a1, a0
	srl				a1, 7
	
	andi			v0, CACHE_TYPE_WAY_MASK

1:
	beqz			v0, 2f				//单路？
	subu			a1, 1
	
	cache 			Hit_Invalidate_D, 16(a0)
	cache 			Hit_Invalidate_D, 48(a0)
	cache 			Hit_Invalidate_D, 80(a0)
	cache 			Hit_Invalidate_D, 112(a0)
	
2:
	cache 			Hit_Invalidate_D, 0(a0)
	cache 			Hit_Invalidate_D, 32(a0)
	cache 			Hit_Invalidate_D, 64(a0)
	cache 			Hit_Invalidate_D, 96(a0)

	bnez			a1, 1b
	addu			a0, 128

3:
	jr				ra
	nop
	.set			reorder
END(cpu_hit_invalidate_dcache)

/**
 * void cpu_hit_invalidate_scache(vm_offset_t addr, vm_size_t len)
 * 描述：命中型作废指定区域内的二级缓存
 * 注意：该函数将对行长作出假设，路行长统一认为是32字节
 * 		 传入的地址参数，要确保为kseg0或者经过tlb映射的虚拟地址，防止发生TLB重填异常
 * 寄存器使用a0-a1
 */
LEAF(cpu_hit_invalidate_scache)
	.set			noreorder

	beqz			a1, 2f
	addu			a1, a0, a1
	addu			a1, 127				//大小向上128字节对齐
	and				a0, -128			//起始地址向下128字节对齐
	subu			a1, a0
	srl				a1, 7
	
1:
	subu			a1, 1
	
	cache 			Hit_Invalidate_S, 0(a0)
	cache 			Hit_Invalidate_S, 32(a0)
	cache 			Hit_Invalidate_S, 64(a0)
	cache 			Hit_Invalidate_S, 96(a0)

	bnez			a1, 1b
	addu			a0, 128

2:
	jr				ra
	nop
	.set			reorder
END(cpu_hit_invalidate_scache)

/**
 * void cpu_flush_cache_io(vm_offset_t addr, vm_size_t len, int rw)
 * 描述：命中型作废或者flush（当addr或len不满足128字节对齐时）指定区域内的所有缓存
 * 参数： 传入的地址参数，要确保为kseg0或者经过tlb映射的虚拟地址，防止发生TLB重填异常；
 *		  当操作的地址范围未按照128字节对齐时，将dcache中的数据作废并写回主存；
 *		  当IO写时，表示外设发起读DMA，此时需要作废并写回对应位置的缓存行，这里假设片外l2以及l3为透写型cache，而不对其进行操作???；
 *		  当IO读时，表示外设发起写DMA，此时需要作废对应位置的缓存行，并且如果DMA区域不满足128字节对齐时，还需要对该区域进行写回
 * 寄存器使用a0-a2, t0, sp
 */
NON_LEAF(cpu_flush_cache_io, FRAME_STAND_SIZE, ra)
	.set			noreorder
	subu			sp, FRAME_STAND_SIZE		//入栈

	sw				ra, FRAME_RA_OFFS(sp)
	sw				a0, FRAME_STAND_SIZE(sp)
	beqz			a2, io_read
	sw				a1, FRAME_STAND_SIZE + 4(sp)
	
io_write:
	/* IO写，外设对存储器发起读DMA操作 */
	lw				t0, cpu_cache_type
	nop
	andi			t0, CACHE_TYPE_HAS_L2	
	/** 
	 * 下面指令使用了两个技巧，首先没有保存ra寄存器值，因此条件满足时执行到cpu_hit_flush_dcache
	 * 后将直接跳转到cpu_flush_cache_io的调用返回地址（等同于当前函数返回），并且使用了可能分支
	 * 指令，防止sp未满足条件的情况下错误出栈，注意此处使用了相对跳转，因此cpu_hit_flush_dcache
 	 * 函数与当前函数的相对偏移不能太远
	 */
	beqzl			t0, cpu_hit_flush_dcache
	addu			sp, FRAME_STAND_SIZE		//出栈
	
	jal				cpu_hit_flush_scache
	nop

	lw				a0, FRAME_STAND_SIZE(sp)
	jal				cpu_hit_flush_dcache
	lw				a1, FRAME_STAND_SIZE + 4(sp)
	
	b				flush_done
	lw				ra, FRAME_RA_OFFS(sp)

io_read:
	/* IO读，外设对存储器发起写DMA操作 */
	andi			t0, a0, 127			//检查addr和len是否128字节对齐
	bnez			t0, flush_rdwb
	andi			t0, a1, 127		
	bnez			t0, flush_rdwb
	nop

	/* 作废数据cache  */
flush_rd:
	lw				t0, cpu_cache_type
	nop
	andi			t0, CACHE_TYPE_HAS_L2	
	bnez			t0, flush_rdl2
	nop
	
	jal				cpu_hit_invalidate_dcache
	nop
	
	b				flush_rdxl2
	lw				ra, FRAME_RA_OFFS(sp)

	/* 作废片内二级cache */
flush_rdl2:
	jal				cpu_hit_invalidate_scache
	nop
	
	lw				a0, FRAME_STAND_SIZE(sp)
	jal				cpu_hit_invalidate_dcache
	lw				a1, FRAME_STAND_SIZE + 4(sp)
	
	b				flush_rdl3
	lw				ra, FRAME_RA_OFFS(sp)

	/* 作废并写回数据cache */
flush_rdwb:
	lw				t0, cpu_cache_type
	nop
	andi			t0, CACHE_TYPE_HAS_L2	
	bnez			t0, flush_rdwbl2
	nop
	
	jal				cpu_hit_flush_dcache
	nop
	
	b				flush_rdxl2
	lw				ra, FRAME_RA_OFFS(sp)

	/* 作废并写回片内二级cache */
flush_rdwbl2:
	jal				cpu_hit_flush_scache
	nop
	
	lw				a0, FRAME_STAND_SIZE(sp)
	jal				cpu_hit_flush_dcache
	lw				a1, FRAME_STAND_SIZE + 4(sp)
	
	b				flush_rdl3
	lw				ra, FRAME_RA_OFFS(sp)

	/* 作废片外二级cache，到这个位置时，ra已经正确设置 */
flush_rdxl2:
	lw				t0, cpu_cache_type
	nop
	andi			t0, CACHE_TYPE_HAS_XL2
	beqz			t0, flush_rdl3
	lw				a0, FRAME_STAND_SIZE(sp)
	lw				a1, FRAME_STAND_SIZE + 4(sp)
	andi			a2, a0, 4095		//基地址对齐
	subu			a0, a2
	addu			a1, a2

1:
	blez			a1, flush_done		//不再进行三级cache操作了????	
	subu			a1, 4096

	cache			Invalidate_Page_S, 0(a0)
	b				1b
	addu			a0, 4096

	/* 作废三级cache，到这个位置时，ra已经正确设置 */
flush_rdl3:
	lw				t0, cpu_cache_type
	nop
	andi			t0, CACHE_TYPE_HAS_L3
	beqz			t0, flush_done
	lw				a0, FRAME_STAND_SIZE(sp)
	lw				a1, FRAME_STAND_SIZE + 4(sp)
	andi			a2, a0, 4095		//基地址对齐
	subu			a0, a2
	addu			a1, a2

2:
	blez			a1, flush_done		//不再进行三级cache操作了????	
	subu			a1, 4096

	cache			Invalidate_Page_T, 0(a0)
	b				2b
	addu			a0, 4096

flush_done:
	/* 跳到此位置时，必须保证返回地址寄存器已经出栈 */
	nop;nop;nop;nop;nop;nop;nop;nop;nop;nop
	jr				ra
	addu			sp, FRAME_STAND_SIZE		//出栈
	.set			reorder
END(cpu_flush_cache_io)

.set pop
.set mips0

