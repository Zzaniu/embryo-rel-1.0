
/**
 * Copyright(c) 2016-3-5 Shangwen Wu	
 *
 * embryo设备初始化通用代码
 * 
 */
#include <mach/early_log.h>
#include <sys/syslog.h>
#include <sys/autoconf.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/system.h>
#include <sys/socket.h>
#include <sys/domain.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/device.h>
#include <mach/intr.h>
#include <mach/clock.h>
#include <env.h>

extern void __init(void);						//defined in ctors.c
extern void mach_devinit_early(void);			//defined in machdep.c
extern void mach_devinit(void);					//defined in machdep.c
extern void kmeminit(int raw_memsize);			//defined in kern_malloc.c

static void pseudo_init(void);

void devinit(int raw_memsize)
{
	int s;

	early_info("init platform devices...\r\n");

	__init();					//调用全局构造函数

	envinit();					//初始化环境变量

	mach_devinit_early();		//具体板子相关的设备早期初始化

	loginit();					//初始化日志系统

	paraminit();				//初始化系统相关参数

	kmeminit(raw_memsize);		//初始化BIOS系统内存空间
	
	mbufinit();					//初始化网络协议栈使用的内存管理空间

	mach_devinit();				//具体板子相关的设备初始化

	mach_clkinit();				//依赖架构相关的时钟初始化

	s = splhigh();

	config_init();
	configure();				//设备初始化配置

	pseudo_init();				//初始化回环网络接口等虚拟设备

	domaininit();				//初始化BIOS支持的网络域

	splx(s);

	procinit();					//初始化进程

	printf("platform devices init done!\r\n");
}

static void pseudo_init(void)
{
	struct pdevinfo *pdev;
	extern struct pdevinfo pdevinfo[]; 

	for(pdev = pdevinfo; pdev->pdev_attach; ++pdev) 
		if(pdev->pdev_cnt > 0) 
			pdev->pdev_attach(pdev->pdev_cnt);
}
