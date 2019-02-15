
/**
 * Copyright(c) 2015-8-26 Shangwen Wu	
 *
 * string函数集实现 
 * 
 */

#include <string.h>
#include <ctype.h>

/**
 * 描述：查找字符串中匹配指定字符的首个指针位置
 */ 
char *strchr(const char *s, int c)
{
	while(*s != (char)c && *s != '\0')
		++s;
	if('\0' == *s)
		return NULL;
	return (char *)s;
}

/**
 * 描述：查找字符串中匹配指定字符的最后一个指针位置
 */ 
char *strlchr(const char *s, int c)
{
	char *l = (char *)s + strlen(s) - 1;	

	while(*l != (char)c && l != s)
		--l;
	if(l == s)
		return NULL;
	return l;
}

/**
 * 描述：字符串拷贝
 */ 
char *strcpy(char *d, const char *s)
{
	char *t = d;

	while((*d++ = *s++) != '\0')
		;
	return t;
}

/**
 * 描述：拷贝指定长度的字符串
 * 注意：当n==0时，将不会对串d进行任何操作
 */
char *strncpy(char *d, const char *s, size_t n)
{
	char *t = d;

	while(n--) {
		if((*d++ = *s) != '\0')
			s++;
	}
	return t;
}

/**
 * 描述：内存拷贝函数
 */
void *memcpy(void *d, const void *s, size_t n)
{
	void *t = d;
	char *d1 = (char *)d, *s1 = (char *)s;

	if(s1 < d1) 
		while(n-- > 0)
			*(d1 + n) = *(s1 + n);
	else
		while(n-- > 0)
			*d1++ = *s1++;

	return t;
}

/**
 * 描述：获取字符串长度
 */
size_t strlen(const char *s)
{
	size_t n = 0;
	
	while(*s++ != '\0')
		++n;
	return n;
}

/**
 * 描述：追加字符串
 */ 
char *strcat(char *d, const char *s)
{
	char *t = d;

	while(*d != '\0')
		++d;
	
	while((*d++ = *s++) != '\0')
		;

	return t;
}

/**
 * 描述：追加指定长度的字符串
 */
char *strncat(char *d, const char *s, size_t n)
{
	char *t = d;

	if(n) {
		while(*d != '\0')
			++d;
		
		while((*d++ = *s++) != '\0') {
			if(0 == --n) {
				*d = '\0';
				break;
			}
		}
	}

	return t;
}

/**
 * 描述：比较两个字符串是否相等
 * 返回：0表示两个字符串相等，不为0，表示两个字符串不相等，
 * 		 返回值为两个字符串最后一个字符的ascii差值
 */
int strcmp(const char *d, const char *s)
{
	while(*d && *s)	
		if(*d++ != *s++)
			return *--d - *--s;

	return *d - *s;
}

/**
 * 描述：比较两个字符指定长度的串是否相等
 * 返回：0表示两个字符串相等，不为0，表示两个字符串不相等，
 * 		 返回值为两个字符串最后一个字符的ascii差值
 */
int strncmp(const char *d, const char *s, size_t n)
{
	while(*d && *s)	
		if(*d++ != *s++ || --n <= 0)
			return *--d - *--s;

	return *d - *s;
}

/**
 * 描述：内存填充
 */
void *memset(void *s, int c, size_t n)
{
	char *t = (char *)s;

	while(n--)	
		*t++ = c;

	return s;
}

/* 非标准字符串操作函数，用于字符串的插入和删除等操作 */

/**
 * 描述：删除字符串中的某个字节，被删除字节的后续字节依次往前移动
 * 参数：p，要删除字节位置的指针
 * 返回：返回删除字节位置的指针
 */
char *strdchr(char *p)
{
	char *t;

	for(t = p; *t; ++t)
		*t = *(t + 1);
	
	return p;
}

/**
 * 描述：插入字符到字符串的指定位置，被插入位置的字节依次往后移动
 * 参数：p，要插入字节位置的指针
 * 返回：返回插入字节位置的指针
 */
char *strichr(char *p, int c)
{
	size_t n;

	for(n = strlen(p) + 1; n > 0; --n)
		p[n] = p[n - 1];
	*p = c;	
	
	return p;
}

/**
 * 描述：插入字符串到字符串的指定位置，被插入位置的字节依次往后移动
 * 参数：p，要插入字符串位置的指针
 * 返回：返回插入字符串位置的指针
 */
char *stristr(char *p, const char *s)
{
	size_t i;
 
	for(i = 0; i < strlen(s); ++i)
		strichr(p + i, s[i]);
	
	return p;
}

/**
 * 描述：字符串是否全由包含空格或制表符组成
 * 返回：满足条件返回1（包括'\0'），否则返回0
 */
int strempty(const char *s)
{
	char *p;

	if(!*s)
		return 1;

	for(p = (char *)s; *p; ++p)
		if(!isspace(*p))
			return 0;

	return 1;
}

/**
 * 描述：源字符串是否是目标字符串的前缀
 * 返回：满足条件返回1，否则返回0
 */
int strprefix(const char *d, const char *s)
{
	for(; *d && *s; ++d, ++s)
		if(*d != *s)
			return 0;
	if(!*s)
		return 1;

	return 0;
}

/**
 * 描述：32位版本内存拷贝函数
 */
void memcpy32(unsigned int *d, unsigned int *s, unsigned int n)
{
	while(n--)
		*d++ = *s++;
}

/**
 * 描述：32位版本内存填充
 */
void memset32(unsigned int *d, int c, size_t n)
{
	while(n--)	
		*d++ = c;
}

/**
 * 描述：字符串匹配，查找字符串d第一次在字符串s中的位置
 * 返回：找到匹配的字符串，返回匹配的首地址，未匹配返回NULL
 */
char *strstr(const char *s, const char *d)
{
	char *p, *q;

	/* 注意：当*d为‘\0’，返回s的起始位置 */
	if(!*d)
		return (char *)(s);

	for(; *s; ++s) {
		if(*s == *d) {
			for(p = (char *)s, q = (char *)d; (*++p == *++q) && *p;)
				;
			if(!*q)
				return (char *)s;
		}
	}

	return NULL;
}

#define MAXLEN	128	

/**
 * 描述：带?通配符的字符串匹配，查找字符串d第一次在字符串s中的位置
 * 返回：找到匹配的字符串，返回匹配的首地址，未匹配返回NULL
 */
static char *__find_first(const char *s, const char *d)
{
	char *p, *q;

	/* 注意：当*d为‘\0’，返回s的起始位置 */
	if(!*d)
		return (char *)(s);

	for(; *s; ++s) {
		if(*s == *d || '?' == *d) {
			for(p = (char *)s, q = (char *)d; ((*++p == *++q) || '?' == *q) && *p;)
				;
			if(!*q)
				return (char *)s;
		}
	}

	return NULL;
}

/**
 * 描述：带?通配符的字符串匹配，查找字符串d最后一次在字符串s中的位置
 * 返回：找到匹配的字符串，返回匹配的首地址，未匹配返回NULL
 */
static char *__find_last(const char *s, const char *d)
{
	char *p, *q, *maybe = NULL;

	/* 注意：当*d为‘\0’，返回s的末尾位置 */
	if(!*d)
		return (char *)(s + strlen(s));

	for(; *s; ++s) {
		if(*s == *d || '?' == *d) {
			for(p = (char *)s, q = (char *)d; ((*++p == *++q) || '?' == *q) && *p;)
				;
			if(!*q) 
				maybe = (char *)s;
		}
	}

	return maybe;
}

/**
 * 描述：通配符匹配字符串，匹配成功返回1，否则返回0
 * 		 该函数可读性和效率都很差，期望以后会有改进版本；该算法整体思路比较简单，
 * 		 主要是以*将带通配符的模式字符串分割成多个小片段，每个片段依次与被比较
 * 		 字符串进行匹配，如果有任何一个片段没有依照固定的顺序出现在被比较字符串
 * 		 中，那么匹配失败。此外对于首尾两端的非*比较字符，需要做特殊处理。
 * 参数：s，源字符串，d，包含通匹符的字符串
 * 注意：暂不支持多个连续**的情况
 */
int strpat(const char *s0, const char *d0)
{
	char *s, *d, *t;
	char temp[MAXLEN], __d[MAXLEN];
	size_t n = strlen(d0), tn;

	/* 在比较字符串的首部和尾部分别加上'^'和'$'用于标识起始和末尾 */
	__d[0] = '^';
	strncpy(__d + 1, d0, n);
	__d[1 + n] = '$';
	__d[2 + n] = '\0';
	
	s = (char *)s0;
	d = (char *)__d;

	while(1) {
		if('^' == *d || '*' == *d) {
			for(t = d + 1; (*t != '*') && (*t != '$'); ++t) 
				;
			tn = t -d -1;
			strncpy(temp, d + 1, tn);
			temp[tn] = 0;
			if('$' == *t) {
				if(!(s = __find_last(s, temp)))
					return 0;
				if('^' == *d && s != s0) 
					return 0;
				if(s + tn != s0 + strlen(s0))
					return 0;
				else
					return 1;
			} else {
				if(!(s = __find_first(s, temp)))
					return 0;
				if('^' == *d && s != s0) 
					return 0;
				s += tn;
				d = t;
			}
		}
	}
	
	return 0;
}

