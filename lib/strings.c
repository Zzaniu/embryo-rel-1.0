
/**
 * Copyright(c) 2016-9-1 Shangwen Wu	
 *
 * strings库遗留函数头文件
 * 
 */

#include <strings.h>

/**
 * 描述：内存清零
 */
void bzero(void *s, size_t n)
{
	char *t = (char *)s;

	while(n--)	
		*t++ = 0;
}

/**
 * 描述：内存拷贝，该函数对拷贝空间叠加的情况做处理
 */
void bcopy(const void *s, void *d, size_t n)
{
	char *bs = (char *)s, *bd = (char *)d;

	if(bs < bd)
		while(n-- > 0)
			*(bd + n) = *(bs + n);
	else 
		while(n-- > 0)
			*bd++ = *bs++;
}

/**
 * 描述：内存比较，比较指定长度的内存
 * 返回：返回(比较长度-第一次比较失败的字节位置）
 */
size_t bcmp(const void *a1, const void *a2, size_t n)
{
	const char *t1, *t2;

	if(!n)
		return 0;
	
	t1 = (const char *)a1;
	t2 = (const char *)a2;

	do {
		if(*t1++ != *t2++)
			break;
	} while (--n);

	return n;
}

