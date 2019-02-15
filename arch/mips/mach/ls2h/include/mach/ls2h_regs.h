
/**
 * Copyright(c) 2015-6-6 Shangwen Wu	
 *
 * LS2H寄存器定义（物理地址）
 * 
 */

#ifndef __MACH_LS2H_REGS_H__
#define __MACH_LS2H_REGS_H__

#define LS2H_REGS_BASE					0xbfd00000			/* 龙芯2H寄存器基址,注意，使用32位兼容模式时，必须使用映射后的地址空间，也可以写成XKPHY格式，0x900000001fd00000 */

/* 时钟配置寄存器 */
#define LS2H_CLOCK_CTRL0				0x220	
#define LS2H_CLOCK_CTRL1				0x224
#define LS2H_CLOCK_CTRL2				0x228
/* 显示控制时钟 */
#define LS2H_PIXCLK0_CTRL_REG_BASE		(LS2H_REGS_BASE + 0x0230)			//DVO
#define LS2H_PIXCLK1_CTRL_REG_BASE		(LS2H_REGS_BASE + 0x0238)			//VGA
#define PIXCLK_CTRL0					0x0			
#define PIXCLK_CTRL1					0x4

/* 分频陪频系数偏移 */
#define	CPUPLL_LDF_OFFSET				1					/* CPUPLL陪频偏移 */
#define CPUPLL_ODF_OFFSET				8					/* CPUPLL输出分频偏移 */
#define DDRPLL_LDF_OFFSET				24					/* DDRPLL陪频偏移 */	
#define DDRPLL_ODF_OFFSET 				22					/* DDRPLL输出分频偏移 */
#define DDRPLL_IDF_OFFSET 				19					/* DDRPLL输入分频偏移 */

#define CPUPLL_SET						0x0001				/* CPUPLL更新位 */
#define CPUPLL_PD						0x0800				/* CPUPLL更新位 */
#define CPUPLL_SEL						0x1000				/* CPULL时钟输出选择 */

#define DDRPLL_SET						0x10000				/* CPUPLL更新位 */
#define DDRPLL_SEL						0x20000				/* CPULL时钟输出选择 */
#define DDRPLL_PD						0x40000				/* CPUPLL更新位 */

/* 芯片采样寄存器 */
#define LS2H_CHIP_SAMPLE0				0x210				
#define LS2H_CHIP_SAMPLE1				0x214
#define LS2H_CHIP_SAMPLE2				0x218
#define LS2H_CHIP_SAMPLE3				0x21c

#define LS2H_CHIP_SAMPLE_REG_BASE		(LS2H_REGS_BASE + 0x210)				
#define LS2H_CHIP_SAMPLE0_REG			(LS2H_CHIP_SAMPLE_REG_BASE + 0x00)
#define LS2H_CHIP_SAMPLE1_REG			(LS2H_CHIP_SAMPLE_REG_BASE + 0x04)
#define LS2H_CHIP_SAMPLE2_REG			(LS2H_CHIP_SAMPLE_REG_BASE + 0x08)
#define LS2H_CHIP_SAMPLE3_REG			(LS2H_CHIP_SAMPLE_REG_BASE + 0x0c)

/* 上电配置位 */
#define BOOTCFG_SYS_CLKSEL0				0x00010000			/* 系统参考时钟选择 */
#define BOOTCFG_SYS_CLKSEL1				0x00020000			/* PCIE参考时钟选择 */
#define BOOTCFG_SYS_CLKSEL2				0x00040000			/* USB PHY1参考时钟选择 */
#define BOOTCFG_SYS_CLKSEL3				0x00080000			/* ACPI参考时钟选择 */
#define BOOTCFG_CPULDF_OFFSET			20					/* CPU PLL倍频系数偏移 */
#define BOOTCFG_CPUPLL_HARD				0x00400000			/* CPU PLL硬件配置 */
#define BOOTCFG_DDRLDF_OFFSET			23					/* DDR PLL陪频系数偏移 */
#define BOOTCFG_DDRPLL_HARD				0x02000000			/* DDR PLL硬件配置 */
#define BOOTCFG_SYSPLL_HARD				0x04000000			/* SYS PLL硬件配置 */

/* 芯片配置模块寄存器组 */
#define LS2H_CHIP_CONFIG0_REG			(LS2H_REGS_BASE + 0x0200)
#define LS2H_CHIP_CONFIG1_REG			(LS2H_REGS_BASE + 0x0204)
#define LS2H_CHIP_CONFIG2_REG			(LS2H_REGS_BASE + 0x0208)
#define LS2H_CHIP_CONFIG3_REG			(LS2H_REGS_BASE + 0x020c)

/* 黑户寄存器 */
#define LS2H_QOS_CFG6_REG				(LS2H_REGS_BASE + 0x80630)

/* GPIO控制模块寄存器*/
#define LS2H_GPIOCFG_REG				(LS2H_REGS_BASE + 0x00c0)

/* 串口模块寄存器 */
#define LS2H_UART_REGS_BASE				(LS2H_REGS_BASE + 0x180000)
#define LS2H_UART0_REGS_BASE			LS2H_UART_REGS_BASE
#define LS2H_UART1_REGS_BASE			(LS2H_UART_REGS_BASE + 0x1000)
#define LS2H_UART2_REGS_BASE			(LS2H_UART_REGS_BASE + 0x2000)
#define LS2H_UART3_REGS_BASE			(LS2H_UART_REGS_BASE + 0x3000)
#define __UART_REGS_BASE(n)				LS2H_##n##_REGS_BASE	
#define	_UART_REGS_BASE(x)				__UART_REGS_BASE(x)	/* 注意宏定义的“#和##优先替换原则 */	

/* I2C模块寄存器 */
#define LS2H_I2C_REGS_BASE				(LS2H_REGS_BASE + 0x190000)
#define LS2H_I2C0_REGS_BASE				LS2H_I2C_REGS_BASE
#define LS2H_I2C1_REGS_BASE				(LS2H_I2C_REGS_BASE + 0x1000)
#define __I2C_REGS_BASE(n)				LS2H_##n##_REGS_BASE	
#define	_I2C_REGS_BASE(x)				__I2C_REGS_BASE(x)	/* 注意宏定义的“#和##优先替换原则 */	

#define I2C_PRERLO						0x00				/* 分频锁存器低字节 */
#define I2C_PRERHI						0x01				/* 分频锁存器高字节 */
#define I2C_CTR							0x02				/* 控制寄存器 */
#define I2C_TXR							0x03				/* 发送数据寄存器 */
#define I2C_RXR							0x03				/* 接收数据寄存器 */
#define I2C_CR							0x04				/* 命令控制寄存器 */
#define I2C_SR							0x04				/* 状态寄存器 */

#define I2C_CTR_EN						0x80				/* 使能I2C模块，否则为操作分频寄存器 */
#define I2C_CTR_IEN						0x40				/* 时能I2C中断 */	
#define I2C_CR_START					0x80				/* 产生起始信号 */
#define I2C_CR_STOP						0x40				/* 产生结束信号 */
#define I2C_CR_RD						0x20				/* 产生读信号 */
#define I2C_CR_WR						0x10				/* 产生写信号 */
#define I2C_CR_NACK						0x08				/* 产生NACK信号 */
#define I2C_CR_LACK						0x01				/* 产生中断应答信号 */
#define I2C_SR_RXACK					0x80				/* 收到ACK */
#define I2C_SR_BUSY						0x40				/* 总线忙 */
#define I2C_SR_AL						0x20				/* 设备失去总线控制权 */
#define I2C_SR_TIP						0x02				/* 正在传输数据 */
#define I2C_SR_IF						0x01				/* 中断标识位 */

/* ACPI电源管理模块寄存器 */
#define LS2H_ACPI_REGS_BASE				(LS2H_REGS_BASE + 0x1f0000)
#define LS2H_ACPI_PM1_STS_REG			(LS2H_ACPI_REGS_BASE + 0x0c)
#define LS2H_ACPI_PM1_CNT_REG			(LS2H_ACPI_REGS_BASE + 0x14)
#define LS2H_ACPI_RST_CNT_REG			(LS2H_ACPI_REGS_BASE + 0x30)

/* RTC模块寄存器 */
#define LS2H_RTC_REGS_BASE				(LS2H_REGS_BASE + 0x1f8000)
#define LS2H_TOY_TRIM_REG				(LS2H_RTC_REGS_BASE + 0x20)	/* 必须软件初始化为0 */
#define LS2H_TOY_WRITE0_REG				(LS2H_RTC_REGS_BASE + 0x24)	/* TOY时间写入寄存器低32位 */
#define LS2H_TOY_WRITE1_REG				(LS2H_RTC_REGS_BASE + 0x28)	/* TOY时间读出寄存器低32位 */
#define LS2H_TOY_READ0_REG				(LS2H_RTC_REGS_BASE + 0x2c)	/* TOY时间写入寄存器高32位 */
#define LS2H_TOY_READ1_REG				(LS2H_RTC_REGS_BASE + 0x30)	/* TOY时间读出寄存器高32位 */
#define LS2H_TOY_MATCH0_REG				(LS2H_RTC_REGS_BASE + 0x34)	/* TOY中断相关寄存器 */
#define LS2H_TOY_MATCH1_REG				(LS2H_RTC_REGS_BASE + 0x38)
#define LS2H_TOY_MATCH2_REG				(LS2H_RTC_REGS_BASE + 0x3c)
#define LS2H_RTC_CTRL_REG				(LS2H_RTC_REGS_BASE + 0x40)	/* RTC模块控制寄存器 */
#define LS2H_RTC_TRIM_REG				(LS2H_RTC_REGS_BASE + 0x60)	/* 必须软件初始化为0 */
#define LS2H_RTC_WRITE0_REG				(LS2H_RTC_REGS_BASE + 0x64)	/* RTC计数器写入寄存器 */
#define LS2H_RTC_READ0_REG				(LS2H_RTC_REGS_BASE + 0x68)	/* RTC计数器读出寄存器 */
#define LS2H_RTC_MATCH0_REG				(LS2H_RTC_REGS_BASE + 0x6c)	/* RTC中断相关寄存器 */
#define LS2H_RTC_MATCH1_REG				(LS2H_RTC_REGS_BASE + 0x70)
#define LS2H_RTC_MATCH2_REG				(LS2H_RTC_REGS_BASE + 0x74)

#define RTC_TOY_YEAR_MASK				0x3ff
#define RTC_TOY_YEAR_SHIFT				0
#define RTC_TOY_MONTH_MASK				0x3f
#define RTC_TOY_MONTH_SHIFT				26
#define RTC_TOY_DAY_MASK				0x1f
#define RTC_TOY_DAY_SHIFT				21
#define RTC_TOY_HOUR_MASK				0x1f
#define RTC_TOY_HOUR_SHIFT				16
#define RTC_TOY_MIN_MASK				0x3f
#define RTC_TOY_MIN_SHIFT				10
#define RTC_TOY_SEC_MASK				0x3f
#define RTC_TOY_SEC_SHIFT				4
#define RTC_TOY_MISEC_MASK				0x0f
#define RTC_TOY_MISEC_SHIFT				0

#define RTC_CTRL_REN					0x2000				/* RTC使能 */
#define RTC_CTRL_TEN					0x0800				/* TOY使能 */
#define RTC_CTRL_EO						0x0100				/* 32.768k晶振使能 */

/* 显示控制模块寄存器 */
#define LS2H_DC_DVO_REGS_BASE			(LS2H_REGS_BASE + 0x151240)
#define LS2H_DC_VGA_REGS_BASE			(LS2H_REGS_BASE + 0x151250)
#define DC_FB_CONF						0x00				/* 帧缓冲配置寄存器 */
#define DC_FB_HWADDR0					0x20				/* 帧缓冲区0地址寄存器 */
#define DC_FB_HWADDR1					0x340				/* 帧缓冲区1地址寄存器 */
#define DC_FB_STRIDE					0x40				/* 帧缓冲跨度寄存器 */
#define DC_FB_ORIGIN					0x60				/* 帧缓冲初始字节寄存器 */
#define DC_DITHER_CONF					0x120				/* 抖动配置寄存器 */
#define DC_DITHER_TAB_LO				0x140				/* 抖动查找表低位寄存器 */
#define DC_DITHER_TAB_HI				0x160				/* 抖动查找表高位寄存器 */
#define DC_PAN_CONF						0x180				/* 液晶面板配置寄存器 */
//#define DC_PAN_TIMING					0x1a0				/* 液晶面板时序寄存器（手册未提到） */
#define DC_HDISPLAY						0x1c0				/* 水平显示宽度寄存器 */
#define DC_HSYNC_CONF					0x1e0				/* 行同步配置寄存器 */
#define DC_VDISPLAY						0x240				/* 垂直显示高度寄存器 */
#define DC_VSYNC_CONF					0x260				/* 场同步配置寄存器 */

#endif /* __MACH_LS2H_REGS_H__ */

