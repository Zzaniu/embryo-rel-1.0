
/**
 * Copyright(c) 2016-11-18 Shangwen Wu	
 *
 * mips架构依赖相关代码 
 * 
 */

#include <common.h>
#include <autoconf.h>
#include <cvt.h>
#include <sys/types.h>
#include <stdio.h>
#include <asm/cpu.h>
#include <asm/cache.h>
#include <mach/trapframe.h>

extern void panic(char *msg);

/* 用于保存cache信息的全局变量, defined in mipsdep.c */
unsigned long cpu_cache_type;					/* cache路数，是否有二三级cache，以及cache类型等信息 */
unsigned long cpu_primary_icache_szie;			/* 一级指令cache大小 */
unsigned long cpu_primary_dcache_szie;			/* 一级数据cache大小 */
unsigned long cpu_primary_icache_line_szie;		/* 一级指令cache行长 */
unsigned long cpu_primary_dcache_line_szie;		/* 一级数据cache行长 */
unsigned long cpu_primary_icache_set_szie;		/* 单路指令cache大小 */
unsigned long cpu_primary_dcache_set_szie;		/* 单路数据cache大小 */
unsigned long cpu_secondry_cache_szie;			/* 二级cache大小 */
unsigned long cpu_tertiary_cache_szie;			/* 三级cache大小 */

struct trapframe *cpuframe = NULL;				//指向具体的CPU引导核，该变量在具体架构的初始化函数中被设置

/**
 * 将字符串转换成当前CPU寄存器位宽大小的整数值
 */
int md_ator(register_t *vp, const char *buf, int base)
{
#if __mips >= 3
	return llatob(vp, buf, base);
#else
	return atob(vp, buf, base);
#endif
}

#if FIX_HEAP_SPACE
static unsigned long md_heap_base = 0;
static unsigned long md_heap_top = 0;
static unsigned long md_allocp = 0;

/**
 * 描述：平台依赖的sbrk函数实现
 */
char *md_sbrk(size_t size)
{
	char *p = NULL;

	if(md_allocp + size <= md_heap_top) {
		p = (char *)md_allocp;					//bad code
		md_allocp += size;
	}
	
	return p;
}

/**
 * 描述：判断某个内存地址是否位于堆空间
 * 注意：参数addr应为虚拟地址
 */
int md_on_heap_space(unsigned long vaddr)
{
	if(VA_TO_PHY(vaddr) >= VA_TO_PHY(md_heap_base) && 
			VA_TO_PHY(vaddr) < VA_TO_PHY(md_heap_top))
		return 1;

	return 0;
}

/**
 * 描述：由于该函数处于ctor段，因此该函数将在__init函数中被调用
 */
static void __attribute__((constructor)) heap_init(void)
{
	md_heap_base = HEAP_BASE;
	md_heap_top = HEAP_SIZE + HEAP_BASE;
	md_allocp = md_heap_base;
}
#endif

/* cache flush函数（索引型flush） */
/**
 * 描述：flush特定类型cache
 */
void flush_cache(int type, void *addr)
{
	switch(type) {
		case FLUSH_TYPE_CACHE: 
			flush_allcache();
			break;
		case FLUSH_TYPE_ICACHE: 
			flush_icache((void *)CPU_KSEG_CACHED, cpu_primary_icache_szie);
			break;
		case FLUSH_TYPE_DCACHE: 
			flush_dcache((void *)CPU_KSEG_CACHED, cpu_primary_dcache_szie);
			break;
		case FLUSH_TYPE_SYNCI: 
			synci_cache((void *)((unsigned long)addr & (~3)), 4);		//目前使用32位指令长度？？？
			break;
	}
}

/**
 * 描述：flush所有cache
 */
void flush_allcache(void)
{
	cpu_flush_cache();
}

/**
 * 描述：flush指定区域的数据cache
 */
void flush_dcache(void *addr, size_t len)
{
	cpu_flush_dcache((vm_offset_t)addr, (vm_size_t)len);
}

/**
 * 描述：flush指定区域的指令cache
 */
void flush_icache(void *addr, size_t len)
{
	cpu_flush_icache((vm_offset_t)addr, (vm_size_t)len);
}

/**
 * 描述：指令同步更新操作
 */
void synci_cache(void *addr, size_t len)
{
	cpu_flush_dcache((vm_offset_t)addr, (vm_size_t)len);
	cpu_flush_icache((vm_offset_t)addr, (vm_size_t)len);
}

/**
 * 描述：加载地址是否合法
 */
int md_load_addr_isvalid(void * start, void *end)
{
	if(1 == 0
#if FIX_HEAP_SPACE
		|| md_on_heap_space((unsigned long)start)
		|| md_on_heap_space((unsigned long)end)
#endif	
	)	
		return 0;

	return 1;
}

/**
 * 描述：设置CPU状态寄存器
 */
register_t md_setsr(register_t val)
{
	register_t old;

	/* 在架构级别的初始化之前调用将引起崩溃 */
	if(!cpuframe)
		panic("cpuframe is null");

	old = cpuframe->sr;
	if(val)
		cpuframe->sr = val;

	return old;
}

/**
 * 用于设置CPU和寄存器的相关接口，这些接口在val参数为0时，可用于读取
 * 寄存器当前设置值，而不更新设置
 */

/**
 * 描述：设置PC取指指针
 */
register_t md_setpc(register_t val)
{
	register_t old;

	if(!cpuframe)
		panic("cpuframe is null");

	old = cpuframe->epc;
	if(val)
		cpuframe->epc = val;

	return old;
}

/**
 * 描述：设置返回link地址指针（知道为什么叫lr吗？^_^）
 */
register_t md_setlr(register_t val)
{
	register_t old;

	if(!cpuframe)
		panic("cpuframe is null");

	old = cpuframe->ra;
	if(val)
		cpuframe->ra = val;

	return old;
}

/**
 * 描述：设置栈指针
 */
register_t md_setstack(register_t val)
{
	register_t old;

	if(!cpuframe)
		panic("cpuframe is null");

	old = cpuframe->sp;
	if(val)
		cpuframe->sp = val;

	return old;
}

/**
 * 描述：设置a0-a3函数调用参数
 */
void md_setargs(register_t arg0, register_t arg1, 
			register_t arg2, register_t arg3)
{
	if(!cpuframe)
		panic("cpuframe is null");

	cpuframe->a0 = arg0;
	cpuframe->a1 = arg1;
	cpuframe->a2 = arg2;
	cpuframe->a3 = arg3;
}

/**
 * 描述：打印CPU寄存器
 */
void md_dump_cpuregs(void)
{	
	printf("CPU Core Regs:\n");
	printf("%-*s%llx\n", 40, "a0", cpuframe->a0);
	printf("%-*s%llx\n", 40, "a1", cpuframe->a1);
	printf("%-*s%llx\n", 40, "a2", cpuframe->a2);
	printf("%-*s%llx\n", 40, "a3", cpuframe->a3);
	printf("%-*s%llx\n", 40, "sp", cpuframe->sp);
	printf("%-*s%llx\n", 40, "pc", cpuframe->epc);
	printf("%-*s%llx\n", 40, "ra", cpuframe->ra);
	printf("%-*s%llx\n", 40, "CP0_SR", cpuframe->sr);
}
