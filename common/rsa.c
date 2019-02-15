
/**
 * Copyright(c) 2016-11-18 Shangwen Wu	
 *
 * 获取字符串中的计算表达式的值
 * 
 */
#include <mach/types.h>
#include <stdio.h>
#include <string.h>

extern int md_ator(register_t *vp, const char *buf, int base); 		//defined in mipsdep.c

int get_rsa_reg(register_t *vp, const char *p)
{
	return md_ator(vp, p, 16);
}

#ifdef HAVE_QUAD
int get_rsa(u_quad_t *vp, const char *p)
#else
int get_rsa(unsigned long *vp, const char *p)
#endif
{
	register_t val;

	if(!get_rsa_reg(&val, p)) {
		*vp = val;
		return 0;
	}
	return -1;
}

uint8_t load_byte(unsigned long addr)
{
	return *((volatile uint8_t *)addr);
}

uint16_t load_half(unsigned long addr)
{
	return *((volatile uint16_t *)addr);
}

uint32_t load_word(unsigned long addr)
{
	return *((volatile uint32_t *)addr);
}

uint64_t load_dword(unsigned long addr)
{
	return *((volatile uint64_t *)addr);
}

register_t load_reg(unsigned long addr)
{
	return *((volatile register_t *)addr);
}

void store_byte(void *a, u_int8_t v)
{
	*(volatile u_int8_t *)a = v;
}

void store_half(void *a, u_int16_t v)
{
	*(volatile u_int16_t *)a = v;
}

void store_word(void *a, u_int32_t v)
{
	*(volatile u_int32_t *)a = v;
}

void store_dword(void *a, u_int64_t v)
{
	*(volatile u_int64_t *)a = v;
}

