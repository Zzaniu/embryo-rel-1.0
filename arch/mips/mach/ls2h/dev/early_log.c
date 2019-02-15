
/**
 * Copyright(c) 2015-10-1 Shangwen Wu	
 *
 * 终端使能之前的串口调试函数
 * 
 */

#include <common.h>
#include <stdarg.h>			//vsprintf

#include <autoconf.h>
#include <mach/types.h>
#include <mach/ls2h_regs.h>
#include <mach/early_log.h>
#include <asm/io.h>
#include <mach/ns16550.h>

#define UART_REGS_BASE			_UART_REGS_BASE(BIOS_CONSOLE_UART)

extern int vsprintf(char *buf, const char *fmt, va_list va);		//defined in vsprintf

unsigned char early_getchar(void)
{
	while(!(readb(NS16550_LSR + UART_REGS_BASE) & NS16550_LSR_DR))
		;
	return readb(NS16550_DAT + UART_REGS_BASE);
}

unsigned char early_putchar(unsigned char c)
{
	while(!(readb(NS16550_LSR + UART_REGS_BASE) & NS16550_LSR_TE))
		;
	writeb(c, NS16550_DAT + UART_REGS_BASE);
	return c;
}

int early_putstr(const char *s)
{
	const char *t = s;
	while(*t)
		early_putchar(*(t++));
	return (t - s);
}

/**
 * 描述：注意当接收到字符'\r'后，函数将返回，
 * 		收到'\0'函数不会返回， 并且会在末尾填充'\0'
 * 返回值：函数将返回接收到的字符个数
 * 注意：该函数不会检测数组越界
 */
int early_getstr(char *s)
{
	int n = 0;
	unsigned char c;

	if(NULL == s)	
		return 0;
	
	while('\r' != (s[n++] = early_getchar()))
		;
	s[n++] = '\n';
	s[n] = '\0';
	return n;
}

/**
 * printf的调试串口版本
 */
int early_printf(const char *fmt, ...)
{
	int n;
	char buf[1024];
	va_list ap;
	
	va_start(ap, fmt);
	n = vsprintf(buf, fmt, ap);
	va_end(ap);

	early_putstr(buf);

	return n;
}

