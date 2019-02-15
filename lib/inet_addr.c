
/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * Internet相关API 
 * 
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* 根据传入的“xxx.xxx.xxx.xxx”格式的字符串，转换成in_addr_t类型的地址 */
in_addr_t inet_addr(const char *cp)
{
	struct in_addr in;	
	
	if(inet_aton(cp, &in)) 
		return in.s_addr;

	return INADDR_NONE;
}

/**
 * 根据传入的“xxx.xxx.xxx.xxx”格式的字符串，转换成in_addr地址结构，
 * 返回非0表示转换成功，否则转换失败
 */
int inet_aton(const char *cp, struct in_addr *inp)
{
	int base, n = 0;
	unsigned long val;
	unsigned char part[3], *pp = part;
	
	if(!cp)
		return 0;

	for(;;) {
		val = 0;
		base = 10;
		if('0' == *cp) {
			if('x' == *++cp || 'X' == *cp)
				base = 16, ++cp;
			else
				base = 8;
		}
		while(isascii(*cp)) {
			if(isdigit(*cp))
				val = val * base + (*cp - '0');
			else if(16 == base && isxdigit(*cp))
				val = (val << 4) + (*cp - (islower(*cp) ? 'a' : 'A') + 10);
			else
				break;	
			++cp;
		}
		if('.' == *cp) {
			if(val > 0xff)
				return 0;
			if(++n >= 4)
				return 0;
			*pp++ = val;
			++cp;
		} else
			break;
	}
	if(*cp && (!isascii(*cp) || !isspace(*cp)))
		return 0;

	switch(n) {
	case 0:						//a(32bits)
		break;
	case 1:						//a.b(24bits)
		if(val > 0xffffff)
			return 0;
		val |= part[0] << 24;
		break;
	case 2:						//a.b.c(16bits)
		if(val > 0xffff)
			return 0;
		val |= (part[0] << 24) | (part[1] << 16);
		break;
	case 3:						//a.b.c.d
		if(val > 0xff)
			return 0;
		val |= (part[0] << 24) | (part[1] << 16) | (part[2] << 8);
		break;
	}

	if(inp) 
		inp->s_addr = (in_addr_t)htonl(val);

	return 1;
}

/* 根据传入的in_addr地址结构体，转换成“xxx.xxx.xxx.xxx”格式的字符串 */
char *inet_ntoa(struct in_addr in)
{
	/* 注意！这里一定要使用static，否则将返回一个被释放的缓冲区指针 */
	static char buf[4 * sizeof("192")];			//xxx.xxx.xxx.xxx
	u_int8_t *ap = (u_int8_t *)&in;
	
	sprintf(buf, "%d.%d.%d.%d", ap[0], ap[1], ap[2], ap[3]);

	return buf;
}

