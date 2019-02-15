
/**
 * Copyright(c) 2015-8-26 Shangwen Wu	
 *
 * embryo-bios C环境入口文件 
 * 
 */

//#include <arch/mips/varargs.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <autoconf.h>
#include <mach/types.h>
#include <asm/io.h>
#include <asm/cpu.h>
#include <asm/cache.h>
#include <mach/bonito.h>
#include <mach/trapframe.h>
#include <mach/ls2h_regs.h>
#include <mach/early_log.h>
#include <logo.h>
#include <shell.h>

#include <mach/pflash.h>				//环境变量相关
#include <flash/flash_map.h>

#include <fs/termio.h>					//交互终端相关

void initmips(int memsize);

static void fpu_enable(void);
void mach_showlogo(void);
void mach_warmreset(void);
void mach_devinit(void);

extern void tlb_clear(void);			//difined in start.S
extern void do_post(int raw_memsize);	//defined in post.c
extern void devinit(int raw_memsize);	//defined in devinit.c
extern struct flash_map *find_flash_map(unsigned long base);
extern int dc_init(void);				//defined in dc.c 
extern int fb_init(unsigned long, unsigned int, unsigned int, unsigned char);	//defiend in fb.c
extern int fbconsole_init(unsigned int col_max, unsigned int raw_max,
						unsigned int bg, unsigned int fg, unsigned bpp);	  	//defined in fb_console.c
extern register_t md_setpc(register_t val);	//defined in mipsdep.c
extern register_t md_setstack(register_t val);
extern register_t md_setsr(register_t val);

extern struct tty_operations n16550_ops; //defined in ns16550.c
extern struct tty_operations fbcon_ops; //defined in fb_console.c

extern unsigned char fb_bbp;			//defined in dc.c
extern unsigned long fb_base;
extern unsigned int fb_xsize, fb_ysize;

int vga_available = 0;					//用于表示当前VGA是否可用
extern struct trapframe bootcore_regsframe;	//用于go命令中设置CPU核寄存器表, defined in mipsdep.c
extern struct trapframe *cpuframe;		//defined in mipsdep.c

/* 以下设备配置表的先后顺序必须先是串口，再是VGA，必须和标准IO的顺序一致 */
struct tty_configure tty_cfgtable[] = {
	{_UART_REGS_BASE(BIOS_CONSOLE_UART), 125000000, CONSOLE_UART_BAUDRATE, &n16550_ops},
	{1UL, 0, 0, &fbcon_ops},					//注意！iobase不能为0
	{},
};

/* 客户程序的默认栈基地址，这里返回非cache地址？？ */
static register_t mach_getclientstack(void)
{
	extern u_int64_t memsize_avail; 			//defined in kern_malloc.c

	/* 从可用内存（低250M）的最高8字节对齐地址向下增长的区域为栈指针 */
	return ((register_t)(long)PHY_TO_UNCACHED(memsize_avail & ~7) - 64);
}

/**
 * 描述：初始化CPU寄存器表
 * 注意：该函数必须在devinit后调用，因为某些参数需要在devinit中
 * 		 先进行设置
 */
static void init_cpuregs(void)
{
	cpuframe = &bootcore_regsframe;
	
	/* 设置CPU为异常级别、并打开CP1和32个浮点寄存器 */
	md_setsr(SR_CU1_ENA | SR_FR32 | SR_EXL);
	/* 设置客户程序（内核）的默认栈指针 */
	md_setstack(mach_getclientstack());
}

/**
 * 描述：与架构相关的初始化函数，并初始化所有的外设
 */
void initmips(int memsize)
{
#if FPU_ENABLE
	fpu_enable();
#endif

	tlb_clear();	

	/* 初始化RTC时钟 */
	mach_rtc_init();

	/* 上电自检测 */
#if POWERON_SELF_TEST
	do_post(memsize);
#endif

	devinit(memsize);

	/* 设置CPU寄存器表，用于go/boot跳转命令 */
	init_cpuregs();

	enter_shell();				//shell.h
}

/************************** 设备初始化 *****************************/
/**
 * 描述：使能浮点运算单元
 * 		 打开SR寄存器的FPU使能开关，并根据指令集兼容类型使能偶数浮点寄存器 
 * 		 清零浮点控制寄存器
 * 注意：__mips为gcc编译选项传递的变量，目前mips为3
 * 		 该函数将导致编译器延迟槽警告
 */ 
static void fpu_enable(void)
{
#if __mips < 3
		asm(\
			"mfc0 	$2, $12;\n" \
			"li		$3, 0x30000000;\n" \
			"or		$2, $3;\n" \
  			"mtc0	$2, $12;\n" \
  			"li		$2, 0x00000000;\n" \
			"ctc1	$2, $31;\n" \
			:::"$2", "$3" \
		);
#else
		asm(\
			"mfc0 	$2, $12;\n" \
			"li		$3, 0x34000000;\n" \
			"or		$2, $3;\n" \
  			"mtc0	$2, $12;\n" \
  			"li		$2, 0x00000000;\n" \
			"ctc1	$2, $31;\n" \
			:::"$2", "$3" \
		);
#endif
}

/************************** 板级相关函数 *****************************/
/**
 * 描述：打印LOGO
 */
void mach_showlogo(void)
{
	int cur = 0;

	while(logo[cur] != '\0') 
		putchar(logo[cur++]);
}

/**
 * 描述：求校验和函数，类似于网络中二字节累加算法
 * 参数：p为传入需要计算校验和二进制序列，n表示这个序列的字节长度
 */
static u16 chksum(u16 *p, int n)
{
	u32 sum = 0;
	u16 *i = p;

	while(n - 2 >= 0) {		
		sum += *i++;
		n -= 2;
	}
	if(1 == n)
		sum += *((u8 *)i);
	sum = (sum >> 16) + (sum & 0xffff);
	while(sum >> 16)
		sum += (sum >> 16);

	return ~sum;
}

/**
 * 描述：将NVRAM中保存的环境变量拷贝到内存中
 */
int mach_mapenv(int (*_setenv)(const char *name, const char *val))
{
	int i;
	u16	chk;
	unsigned char buf[256];
	unsigned char *ep = NULL, *bp = buf, *vp = NULL;	
	unsigned long env_base; 
	struct flash_map *map = NULL;
	if(NULL == (map = find_flash_map(get_nvram_base())))	
		return -1;
	
	//读取NVRAM地址中的环境变量，并保存到数组中
	env_base = map->flm_base + NVRAM_ENVOFFS;
	early_info("Env nvram base address: 0x%x\r\n", env_base);
	chk = *(u16 *)env_base;			//头两个字节存放校验和
	if(chk != chksum((u16 *)(env_base + 2), NVRAM_ENVSIZE - 2)) {
		early_warnning("env nvram checksum not correct!\r\n");
		//return -2;
	} else {
		ep = (unsigned char *)env_base + 2;
		while((unsigned long)ep < env_base + NVRAM_ENVSIZE - 1) {
			vp = NULL;
			bp = buf;
			i = 256;
			while(i-- && (*bp++ = *ep++) && ((unsigned long)ep < env_base + NVRAM_ENVSIZE - 1)) {
				if('=' == *(bp - 1) && vp != NULL) {
					*(bp - 1) = '\0';
					vp = bp;
				}
			}	
			if(((unsigned long)ep < env_base + NVRAM_ENVSIZE - 1) && (i > 0) && vp) { 
				_setenv(buf, vp);				
			} else if(!vp) {
				;						//skip zero
			} else {
				//出错，键值对超过256字节，或者当前建值到达NVRAM_ENVSIZE边界
				return -3;
			}
		}
	}

	return 0;
}

/**
 * 描述：2H CPU复位函数
 */
void mach_warmreset(void)
{
	writel(0x01, LS2H_ACPI_RST_CNT_REG);
}

/**
 * 描述：2H CPU关机函数
 */
void mach_shutdown(void)
{	
	writel((readl(LS2H_ACPI_PM1_STS_REG) & (~(1 << 15))), LS2H_ACPI_PM1_STS_REG);
	writel(0x3c00, LS2H_ACPI_PM1_CNT_REG);
}

/**
 * 描述：显示初始化
 */
static int mach_display_init(void)
{
	if(dc_init())
		goto failed;

	if(fb_init(fb_base, fb_xsize, fb_ysize, fb_bbp))
		goto failed;

	if(fbconsole_init(fb_xsize, fb_ysize, FB_CONSOLE_BG, FB_CONSOLE_FG, fb_bbp))
		goto failed;

	return 0;

failed:
	return -1;
}

/**
 * 描述：板级早期初始化
 */
void mach_devinit_early(void)
{
	cpu_cache_conf();	
}

/**
 * 描述：2H板级初始化代码
 *
 */
void mach_devinit(void)
{
	int rc;
	
	rc = mach_display_init();
	if(!rc) {
		vga_available = 1;
		printf("Framebuffer init down, display is available now\n");
	}

}
