
/**
 * Copyright(c) 2015-8-26 Shangwen Wu	
 *
 * vsprintf函数实现 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cvt.h>

#define FMT_RJUST 	0							//输出右对齐（默认）
#define FMT_LJUST	1							//输出左对齐
#define FMT_RJUST0	2							//输出右对齐并且高位补0
#define FMT_CENTER	3							//输出居中对齐

static void str_fmt(char *buf, int width, int trunc, int align);
 
/**
 * 描述：sprintf可变参数实现
 */ 
int vsprintf(char *buf, const char *fmt, va_list va)
{
	char *p = NULL, *t = NULL, digitbuf[20] = {0};
	int base, width, trunc, align = FMT_RJUST, haddot, longlong;
	unsigned long digit;

	if(NULL == fmt)
		return 0;
	while(*fmt != '\0') { 
		if('%' == *fmt) {													//格式字符
			haddot = trunc = width = longlong = 0;
			align = FMT_RJUST;
			while(*++fmt != '\0') {
				if(NULL == strchr("cdefgiopsuxEGX%", *fmt)) {				//修饰格式符
					switch(*fmt) {
						case '-':
							align = FMT_LJUST;
						break;
						case '*':
							if(haddot)
								trunc = va_arg(va, int);
							else
								width = va_arg(va, int);
						break;
						case '~':
							align = FMT_CENTER;
						break;
						case '0':
							if(FMT_RJUST == align)
								align = FMT_RJUST0;
						break;
						case '.':
							haddot = 1;
						break;
						case 'l':
							if('l' == *(fmt + 1)) {		//%ll?
								longlong = 1;
								++fmt;
							}
						break;
						default:
							if(isdigit(*fmt)) {								//数字修饰符	
								for(t = (char *)fmt; isdigit(*fmt); ++fmt)
									;
								strncpy(digitbuf, t, fmt - t);
								atob(&digit, digitbuf, 10);
								if(haddot)
									trunc = digit; 
								else
									width = digit;
								--fmt;
							}
							//除以上特殊字符外，其他字符直接丢弃
					}	
				} else {													//数据转换符
					break;
				}
			}
			//修饰格式符处理
			//
			if('\0' == *fmt) {
				continue;
			} else if('%' == *fmt) {
				*buf = '%';
				*(buf + 1) = '\0';
			} else if('c' == *fmt) {
				*buf = (char)va_arg(va, int);
				*(buf + 1) = '\0';
			} else if('s' == *fmt) {
				if((p = (char *)va_arg(va, char *)) != NULL)
					strcpy(buf, p);
				else
					strcpy(buf, "(null)");
			} else {
				if(strchr("diopuxX", *fmt) != NULL) {//整数
					if('d' == *fmt || 'i' == *fmt) 
						base = -10;
					else if('u' == *fmt)
						base = 10;
					else if('x' == *fmt)	
						base = 16;
					else if('X' == *fmt)
						base = -16;
					else if('o' == *fmt)
						base = 8;
					else if('p' == *fmt) {
						base = 16;
						*buf++ = '0';
						*buf++ = 'x';
					}
					if(longlong) {
#ifdef HAVE_QUAD
						llbtoa(buf, va_arg(va, quad_t), base);
#else
						unsigned long x = (unsigned long)va_arg(va, long long);
						btoa(buf, x, base);
#endif
					} else
						btoa(buf, va_arg(va, int), base);
				} else if(strchr("efgEG", *fmt) != NULL) {//浮点数
					if('f' == *fmt)
						;//暂不支持
				}
			}
				
			str_fmt(buf, width, trunc, align); 
			//buf指针移动到当前参数末尾的位置
			while(*buf != '\0')
				++buf;
			++fmt;
		} else {													//一般字符
			*buf++ = *fmt++;
		}
	}
	*buf = '\0';
	return 0;
}

/**
 * 描述：格式化输出字符串
 * 参数：buf为输出缓冲区,调用该函数时，buf应当指向当前格式化数据起
 * 		 始位置,width/trunc/align为当前数据输出宽度/截取长度/对齐方式
 */
static void str_fmt(char *buf, int width, int trunc, int align) 
{
	int i, len = width - strlen(buf), m, n;	
		
	switch(align) {
		case FMT_LJUST:
			for(i = 0; i < len; ++i)
				strcat(buf, " ");
		break;
		case FMT_RJUST:
			for(i = 0; i < len; ++i)
				strichr(buf, ' ');
		break;
		case FMT_RJUST0:
			for(i = 0; i < len; ++i)
				strichr(buf, '0');
		break;
		case FMT_CENTER:
			m = len / 2;
			n = len - m;
			for(i = 0; i < m; ++i)
				strichr(buf, ' ');
			for(i = 0; i < n; ++i)
				strcat(buf, " ");
		break;
	}
}
