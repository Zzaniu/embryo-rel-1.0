
/**
 * Copyright(c) 2015-9-30 Shangwen Wu	
 *
 * 板级IO相关头文件
 * 
 */

#ifndef __ASM_IO_H__
#define __ASM_IO_H__

#define inl(a)		(*((volatile u32 *)(a)))
#define inb(a)		(*((volatile u8 *)(a)))
#define outl(a, v)	(*(volatile u32 *)(a) = (v))
#define outb(a, v)	(*(volatile u8 *)(a) = (v))

static inline u8 readb(const unsigned long addr)
{
	return *(const volatile u8 *)addr;
}

static inline u8 writeb(u8 b, unsigned long addr)
{
	*(volatile u8 *)addr = b;
}

static inline u32 readl(const unsigned long addr)
{
	return *(const volatile u32 *)addr;
}

static inline u32 writel(u32 b, unsigned long addr)
{
	*(volatile u32 *)addr = b;
}

static inline u32 writel_with_flush(u32 b, unsigned long addr)
{
	*(volatile u32 *)addr = b;
	b = *(volatile u32 *)addr;		//flush
}

#endif //__ASM_IO_H__
