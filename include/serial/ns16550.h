
/**
 * Copyright(c) 2015-6-6 Shangwen Wu	
 *
 * ns16550串口兼容芯片寄存器定义
 * 
 */

#ifndef __NS16550_H__
#define __NS16550_H__

#define NS16550_DAT					0x00				/* 数据寄存器 */
#define NS16550_IER  				0x01				/* 中断时能寄存器 */
#define NS16550_IIR  				0x02				/* 中断标识寄存器 */
#define NS16550_FCR  				0x02				/* FIFO控制寄存器 */	
#define NS16550_LCR  				0x03				/* 线路控制寄存器 */
#define NS16550_MCR					0x04				/* MODEN控制寄存器 */
#define NS16550_LSR					0x05				/* 线路状态寄存器 */
#define NS16550_MSR					0x06				/* MODEN状态寄存器 */
#define NS16550_PRELO				0x00				/* 预分频寄存器低8位 */
#define NS16550_PREHI				0x01				/* 预分频寄存器高8位 */

#define NS16550_FCR_TL_1BYTE		0x00				/* FIFO中断触发值 */
#define NS16550_FCR_TL_4BYTE		0x40
#define NS16550_FCR_TL_8BYTE		0x80
#define NS16550_FCR_TL_14BYTE		0xc0
#define NS16550_FCR_TXRST			0x04				/* 发送FIFO复位 */
#define NS16550_FCR_RXRST			0x02				/* 接收FIFO复位 */
#define NS16550_FCR_FIFO			0x01				/* FIFO使能 */

#define NS16550_LCR_DLAB			0x80				/* 访问分频寄存器 */
#define NS16550	_LCR_SPB			0x20				/* 校验位 */
#define NS16550_LCR_CPS				0x10				/* 偶校验 */
#define NS16550_LCR_PC				0x08				/* 使能校验位 */
#define NS16550_LCR_SB				0x04				/* 停止位 */
#define NS16550_LCR_BCC_5BIT		0x00				/* 数据位 */			
#define NS16550_LCR_BCC_6BIT		0x01				
#define NS16550_LCR_BCC_7BIT		0x02				
#define NS16550_LCR_BCC_8BIT		0x03				

#define NS16550_MCR_LOOP			0x10				/* 回环模式 */
#define NS16550_MCR_RTSC			0x02
#define NS16550_MCR_DTRC			0x01

#define NS16550_MSR_CCTS			0x10

#define NS16550_LSR_ERROR			0x80				/* 出错 */
#define NS16550_LSR_TE				0x40				/* 传输FIFO和移位寄存器为空 */
#define NS16550_LSR_TFE				0x20				/* 传输FIFO为空 */
#define NS16550_LSR_BI				0x10				/* 传输被打断 */
#define NS16550_LSR_FE				0x08				/* 接收到没有停止位的帧 */
#define NS16550_LSR_PE				0x04				/* 奇偶校验错误位 */
#define NS16550_LSR_OE				0x02				/* 数据溢出 */
#define NS16550_LSR_DR				0x01				/* FIFO有数据 */

#define nsreg(r) unsigned char r

typedef struct ns16550regs {
	nsreg(dat);
#define prelo dat
	nsreg(ier);
#define prehi ier
	nsreg(iir);
#define fcr iir
	nsreg(lcr);
	nsreg(mcr);
	nsreg(lsr);
	nsreg(msr);
} Ns16550regs;

#endif /* __NS16550_H__ */

