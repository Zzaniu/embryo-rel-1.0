
/**
 * Copyright(c) 2018-8-22 Shangwen Wu	
 *
 * MIPS cache操作定义
 * 
 */

#ifndef __ASM_CACHE_H__
#define	__ASM_CACHE_H__

#define L1_CACHE_SHIFT		5
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)
#define L1_CACHE_MASK		(L1_CACHE_BYTES - 1)

#define L1_CACHE_ALIGN(x)	(((x) + L1_CACHE_MASK) & ~L1_CACHE_MASK)

#ifndef _LOCORE
/* 用于保存cache信息的全局变量, defined in mipsdep.c */
extern unsigned long cpu_cache_type;
extern unsigned long cpu_primary_icache_szie;
extern unsigned long cpu_primary_dcache_szie;
extern unsigned long cpu_primary_icache_line_szie;
extern unsigned long cpu_primary_dcache_line_szie;
extern unsigned long cpu_primary_icache_set_szie;
extern unsigned long cpu_primary_dcache_set_szie;
extern unsigned long cpu_secondry_cache_szie;
extern unsigned long cpu_tertiary_cache_szie;

#define SYNC_W	1	//同步IO写操作
#define SYNC_R	0	//同步IO读操作

extern void cpu_cache_conf(void);	//defined in cache.S
extern void cpu_flush_cache(void);
extern void cpu_flush_icache(vm_offset_t addr, vm_size_t len);
extern void cpu_flush_dcache(vm_offset_t addr, vm_size_t len);
extern void cpu_hit_flush_dcache(vm_offset_t addr, vm_size_t len);
extern void cpu_hit_flush_scache(vm_offset_t addr, vm_size_t len);
extern void cpu_hit_invalidate_dcache(vm_offset_t addr, vm_size_t len);
extern void cpu_hit_invalidate_scache(vm_offset_t addr, vm_size_t len);
extern void cpu_flush_cache_io(vm_offset_t addr, vm_size_t len, int rw);

/* 用于flush cache操作的宏定义 */
#define FLUSH_TYPE_DCACHE		0			/* flush一级数据cache */
#define FLUSH_TYPE_ICACHE		1			/* flush一级指令cache */
#define FLUSH_TYPE_CACHE		2			/* flush所有cache */
#define FLUSH_TYPE_SYNCI		3			/* 同步指令更新，操作包括写回作废当前数据缓存，并作废当前指令缓存 */

extern  void flush_cache(int type, void *addr);
extern  void flush_allcache(void);
extern  void flush_dcache(void *addr, size_t len);
extern  void flush_icache(void *addr, size_t len);
extern  void synci_cache(void *addr, size_t len);

#endif

#endif /* __ASM_CACHE_H__ */

