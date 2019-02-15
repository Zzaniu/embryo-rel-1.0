
/**
 * Copyright(c) 2016-8-16 Shangwen Wu	
 *
 * NS16550兼容串口芯片驱动
 * 
 */
#include <mach/types.h>

#include <serial/ns16550.h>
#include <fs/termio.h>

#include <asm/io.h>
#include <mach/early_log.h>

static int ns16550_init(struct tty_device *dev)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;
	unsigned short clk = dev->clk / 16 / dev->t.o_speed;

	/* 使能fifo，清空fifo，设置4字节触发 */
	outb(&ns16550->fcr, NS16550_FCR_FIFO|NS16550_FCR_TL_4BYTE|NS16550_FCR_TXRST|NS16550_FCR_RXRST);
	/* 设置分频寄存器 */
	outb(&ns16550->lcr, NS16550_LCR_DLAB);
	outb(&ns16550->prelo, clk & 0xff);
	outb(&ns16550->prehi, (clk >> 8) & 0xff);
	/* 当软件流控未打开时，将设置硬件流控 */
	if(!(dev->t.i_flags & TIERXFC) && !(dev->t.o_flags & TOETXFC)) 
		outb(&ns16550->mcr, NS16550_MCR_RTSC|NS16550_MCR_DTRC);
	/* 设置标注串口设置8n1 */
	outb(&ns16550->lcr, NS16550_LCR_BCC_8BIT);
	/* 禁止所有中断 */
	outb(&ns16550->ier, 0x0);

	return 0;
}

static int ns16550_open(struct tty_device *dev)
{
	return 0;				
}

static int ns16550_reset(struct tty_device *dev)
{
	return 0;
}

static int ns16550_setbaud(struct tty_device *dev, int baud)
{
	static int bauds[] = {50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 
				2400, 4800, 9600, 19200, 38400, 57600, 115200, 0};
	int *p, timeout = 1000000;
	unsigned short clk;
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	for(p = bauds; *p != 0; ++p)
		if(*p == baud)
			break;
	if(!*p)				/* 波特率值非法 */
		return -1;
	
	/* 等待fifo和移位寄存器中的数据发送完 */
	while(!(inb(&ns16550->lsr) & NS16550_LSR_TE))
		if(!--timeout)
			break;

	clk = dev->clk / 16 / baud;
	/* 使能fifo，清空fifo，设置4字节触发 */
	outb(&ns16550->fcr, NS16550_FCR_FIFO|NS16550_FCR_TL_4BYTE|NS16550_FCR_TXRST|NS16550_FCR_RXRST);
	/* 设置分频寄存器 */
	outb(&ns16550->lcr, NS16550_LCR_DLAB);
	outb(&ns16550->prelo, clk & 0xff);
	outb(&ns16550->prehi, (clk >> 8) & 0xff);
	/* 当软件流控未打开时，将设置硬件流控 */
	if(!(dev->t.i_flags & TIERXFC) && !(dev->t.o_flags & TOETXFC)) 
		outb(&ns16550->mcr, NS16550_MCR_RTSC|NS16550_MCR_DTRC);
	/* 设置标注串口设置8n1 */
	outb(&ns16550->lcr, NS16550_LCR_BCC_8BIT);
	/* 禁止所有中断 */
	outb(&ns16550->ier, 0x0);

	dev->t.o_speed = baud;
	dev->t.i_speed = baud;

	return 0;
}

static int ns16550_tx_ready(struct tty_device *dev)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	return ((inb(&ns16550->lsr) & NS16550_LSR_TFE));
}

static int ns16550_tx_byte(struct tty_device *dev, unsigned char data)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	outb(&ns16550->dat, data);
	return 0;
}

static int ns16550_rx_ready(struct tty_device *dev)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	return (inb(&ns16550->lsr) & NS16550_LSR_DR);
}

static int ns16550_rx_byte(struct tty_device *dev, unsigned char *data)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	*data = inb(&ns16550->dat);
	return 0;
}

static int ns16550_rx_hardflwctl(struct tty_device *dev, int rx_stop)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	if(rx_stop) 
		outb(&ns16550->mcr, inb(&ns16550->mcr) & ~NS16550_MCR_RTSC);
	else
		outb(&ns16550->mcr, inb(&ns16550->mcr) | NS16550_MCR_RTSC);

	return 0;
}

static int ns16550_tx_hardflwctl(struct tty_device *dev, int *tx_ready)
{
	volatile struct ns16550regs *ns16550 = (volatile struct ns16550regs *)dev->iobase;

	if(inb(&ns16550->msr) & NS16550_MSR_CCTS) 
		*tx_ready = 0;
	else
		*tx_ready = 1;

	return 0;
}

/* NS16550的终端接口函数定义 */
struct tty_operations n16550_ops = {
	.tty_device_init = ns16550_init,
	.tty_device_open = ns16550_open,
	.tty_device_reset = ns16550_reset,
	.tty_set_baudrate = ns16550_setbaud,
	.tty_tx_ready = ns16550_tx_ready,
	.tty_tx_byte = ns16550_tx_byte,
	.tty_rx_ready = ns16550_rx_ready,
	.tty_rx_byte = ns16550_rx_byte,
	.tty_rx_hardflwctl = ns16550_rx_hardflwctl,
	.tty_tx_hardflwctl = ns16550_tx_hardflwctl,
};

