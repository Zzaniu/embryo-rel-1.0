
/**
 * Copyright(c) 2016-8-24 Shangwen Wu	
 *
 * stdio.h头文件
 * 
 */

#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <sys/types.h>

typedef struct file_s {
	int fd;
	int valid;
	int ungetcflag;
	int ungetchar;
}FILE;

#define OPEN_MAX		64			//实际上该上限值会受到内核态的NFILE定义约束

extern FILE __iob[]; 				//defined in stdio.c
extern int vga_available; 			//defined in machdep.c

#define stdin	(&__iob[0])
#if 0
#define stdout	(&__iob[1])
#define stderr	(&__iob[2])
#else
#define stdout	(vga_available ? &__iob[4] : &__iob[1])
#define stderr	(vga_available ? &__iob[4] : &__iob[2])
#endif

#define EOF	(-1)

/* 标准IO文件描述符宏定义 */
#define STDIN			0
#if 0
#define STDOUT			1
#define STDERR			2
#else
#define STDOUT			(vga_available ? 4 : 1)
#define STDERR			(vga_available ? 4 : 2)
#endif

extern int vsprintf(char *buf, const char *fmt, va_list va);
extern int vprintf(const char *fmt, va_list ap);
extern int vfprintf(FILE *stream, const char *fmt, va_list ap);
extern int printf(const char *fmt, ...);
extern int fprintf(FILE *fp, const char *fmt, ...);
extern int sprintf(char *buf, const char *fmt, ...);
extern int fputs(const char *s, FILE *fp);
extern int puts(const char *s);
extern int putchar(int c);
extern int putc(int c, FILE *stream);
extern int fputc(int c, FILE *stream);
extern int ungetc(int c, FILE *stream);
extern int getchar(void);
extern int getc(FILE *stream);
extern int fgetc(FILE *stream);

#endif //__STDIO_H__
