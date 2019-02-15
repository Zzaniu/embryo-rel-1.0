
/**
 * Copyright(c) 2016-2-1 Shangwen Wu	
 *
 * embryo上电自检程序（POST） 
 * 
 */
#include <mach/types.h>
#include <mach/ls2h_regs.h>
#include <asm/io.h>
#include <mach/early_log.h>
#include <mach/time.h>

/* 全局变量 */
static int g_probeerr = 0;
static unsigned int g_cpupipefreq = 0;
static unsigned int g_cpubusfreq = 0;

extern unsigned long get_cpu_count(void); //defined in mips.S
unsigned long probe_cpu_pipe_frequency(void);
unsigned long probe_cpu_bus_frequency(void);
void __probe_cpu_frequency(void);

/**
 * 描述：上电自检程序
 * 参数：raw_memsize 原始内存参数大小
 */
void do_post(int raw_memsize)
{
	int ret, memsize;
	unsigned long cpupipefreq, cpubusfreq;
	struct time_struct t;

	early_info("Poweron Self Test:\r\n");
	/* 读取RTC时间 */
	mach_get_current_time(&t);
	early_info("rtc-time %u-%u-%u %u:%u:%u.%u\r\n",
			t.year, t.month, t.day, t.hour, t.min, t.sec, t.misec);

	cpupipefreq = probe_cpu_pipe_frequency();
	cpubusfreq = probe_cpu_bus_frequency();

	if(g_probeerr) {
		early_info("CPU frequency probe failed\r\n");
	} else {
		early_info("CPU pipe frequency: %uHZ\r\n", cpupipefreq);
		early_info("CPU bus frequency: %uHZ\r\n", cpubusfreq);
	}
	
	//raw_memsize的位域定义见ddr_config_define.h文件，同s1寄存器的内存大小
	memsize = (raw_memsize  & 0xff) * 512;
	early_info("Memory Size: %dMB\r\n", memsize);

}

/**
 * 描述：CPU流水线频率探测函数
 */
unsigned long probe_cpu_pipe_frequency(void)
{
	if(0 == g_cpupipefreq) 
		__probe_cpu_frequency();
	return g_cpupipefreq;
}

/**
 * 描述：CPU总线频率探测函数
 */
unsigned long probe_cpu_bus_frequency(void)
{
	if(0 == g_cpubusfreq) 
		__probe_cpu_frequency();
	return g_cpubusfreq;
}

/**
 * 描述：CPU频率探测子函数
 * 参数：freq 获取的CPU流水线时钟
 * 返回: 获取是否成功
 */
void __probe_cpu_frequency(void)
{
	/* 这里要求系统先初始化RTC */
	int i = 2, sec;
	unsigned long cur, pre, sub;
	unsigned long timeout = 10000000;

	/* 
 	 * 这里需要进行操作两次，第一次保证代码运行在cache，以减小软件
 	 * 给检测造成的误差，第二次实际进行检测
	 */
	while(i--) {
		sec = (inl(LS2H_TOY_READ0_REG) >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK;
		while(--timeout && sec == ((inl(LS2H_TOY_READ0_REG) >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK))
			;
		if(!timeout)
			break;
		pre = get_cpu_count();
		timeout = 10000000;
		sec = (inl(LS2H_TOY_READ0_REG) >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK;
		while(--timeout && sec == ((inl(LS2H_TOY_READ0_REG) >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK))
			;
		if(!timeout)
			break;
		cur = get_cpu_count();
		sub = cur > pre ? cur - pre : 0xffffffff - pre + cur;//注意CP0_COUNT是32位寄存器，这里检测溢出的情况
	}
	if(!timeout) {
		g_probeerr = 1;
		early_err("probe frequency timeout!\r\n");
	}

	g_cpupipefreq = sub / 10000;		//去掉尾数
	g_cpupipefreq *= 20000;				//2h的流水线时钟为COUNT计数器频率的一倍
	g_cpubusfreq = 66000000;			//bad code, 定死了66M
}
