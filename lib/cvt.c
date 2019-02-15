
/**
 * Copyright(c) 2015-10-10 Shangwen Wu	
 *
 * 实现C数据类型之间的转换的库函数集 
 * 
 */

#include <autoconf.h>
#include <cvt.h>

#if SOFT_64BIT_DIV_MOD
uint32_t __div64_u32(uint64_t *n, uint32_t base); 		//defined in div64.c
#endif

/**
 * 描述：将指定进制数转换成字符串（仅用于32位整数）
 * 返回：调用者分配的目标字符串缓冲区
 */ 
char *btoa(char *buf, unsigned long value, int base)
{
	char tmp[34], digit;
	int neg = 0, upper = 0, i = 0;

	if(0 == value) {
		*buf = '0';
		*(buf + 1) = '\0';
		return buf;
	}

	if(-10 == base)	{			//'u'
		if(value & (1L << 31)) {
			value = ~value + 1;
			neg = 1;
		}
		base = 10;
	}

	if(-16 == base)	{			//'X'
		upper = 1;	
		base = 16;
	}
	
	while(value) { 
		digit = value % base;
		if(digit >= 0x0a) {
			digit -= 0x0a;
			if(upper)
				digit += 'A';
			else
				digit += 'a';
		} else 
			digit += '0';
		tmp[i++] = digit;
		value /= base;
	}
	
	if(neg) 
		tmp[i++] = '-';
	//反转字符串
	while(i--) {
		*buf++ = tmp[i];
	}

	*buf = '\0';

	return buf;
}

#ifdef HAVE_QUAD
/**
 * 描述：将指定进制数转换成字符串（btoa 64位版本）
 * 返回：调用者分配的目标字符串缓冲区
 */ 
char *llbtoa(char *buf, u_quad_t value, int base)
{
	char tmp[66], digit;
	int neg = 0, upper = 0, i = 0;

	if(0 == value) {
		*buf = '0';
		*(buf + 1) = '\0';
		return buf;
	}

	if(-10 == base)	{			//'d'
		if(value & (1LL << 63)) {
			value = ~value + 1;
			neg = 1;
		}
		base = 10;
	}

	if(-16 == base)	{			//'X'
		upper = 1;	
		base = 16;
	}
	
	while(value) { 
#if SOFT_64BIT_DIV_MOD			/* 使用软件64位除法和求模运算 */
		digit = __div64_u32(&value, base);
#else
		digit = value % base;
		value /= base;
#endif
		if(digit >= 0x0a) {
			digit -= 0x0a;
			if(upper)
				digit += 'A';
			else
				digit += 'a';
		} else 
			digit += '0';
		tmp[i++] = digit;
	}

	if(neg) 
		tmp[i++] = '-';
	//反转字符串
	while(i--) {
		*buf++ = tmp[i];
	}

	*buf = '\0';

	return buf;
}
#endif

#ifdef HAVE_QUAD
static int _atob(u_quad_t *vp, const char *buf, int base)
#else
static int _atob(unsigned long *vp, const char *buf, int base)
#endif
{
	int digit;
	char *p = (char *)buf;
#ifdef HAVE_QUAD
	u_quad_t val;
#else
	unsigned long val;
#endif

	val = *vp = 0;
	if(!*p)
		return -1;
	for(; *p ; ++p) {
		if(*p >= '0' && *p <= '9')
			digit = *p - '0';
		else if(*p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if(*p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else
			return -1;
		if(digit >= base) 
			return -1;
		val *= base;
		val += digit;
	}

	*vp = val;
	return 0;
}

/**
 * 描述：将字符串转换成指定进制数（仅用于32位整数）
 * 参数：vp表示返回转换的结果，buf待转换的字符串，base转换进制数
 * 返回：成功返回0，否则返回-1
 * 注意：传入参数base的值不符合要求时可能出现意想不到的错误bad codes
 */
int atob(unsigned long *vp, const char *buf, int base)
{
	int native = 0;
	char *p = (char *)buf;
#ifdef HAVE_QUAD
	u_quad_t val;
#else
	unsigned long val;
#endif

	if('0' == buf[0] && ('x' == p[1] || 'X' == p[1])) {
		base = 16;
		p += 2;
	}
	else if('o' == buf[0]) {
		base = 8;
		p += 1;
	}
	else if('-' == buf[0]) {
		base = -10;
		p += 1;
	}

	if(!base)
		base = 10;

	if(base < 0) {
		base = -base;
		native = 1;
	}
		
	if(_atob(&val, p, base))
		return -1;

	if(native) 
		val = ~val + 1;

	*vp = val;

	return 0;
}

#ifdef HAVE_QUAD
/**
 * 描述：将字符串转换成指定进制数（64位整数）
 * 参数：vp表示返回转换的结果，buf待转换的字符串，base转换进制数
 * 返回：成功返回0，否则返回-1
 * 注意：传入参数base的值不符合要求时可能出现意想不到的错误bad codes
 */
int llatob(u_quad_t *vp, const char *buf, int base)
{
	int native = 0;
	char *p = (char *)buf;
	u_quad_t val;

	if('0' == buf[0] && ('x' == p[1] || 'X' == p[1])) {
		base = 16;
		p += 2;
	}
	else if('o' == buf[0]) {
		base = 8;
		p += 1;
	}
	else if('-' == buf[0]) {
		base = -10;
		p += 1;
	}

	if(!base)
		base = 10;

	if(base < 0) {
		base = -base;
		native = 1;
	}
		
	if(_atob(&val, p, base))
		return -1;

	if(native) 
		val = ~val + 1;

	*vp = val;

	return 0;
}
#endif
