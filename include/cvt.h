
/**
 * Copyright(c) 2015-10-10 Shangwen Wu	
 *
 * C数据类型转换头文件
 * 
 */

#ifndef __CONVERT_H__
#define __CONVERT_H__

#include <mach/types.h>

extern char *btoa(char *buf, unsigned long value, int base);
#ifdef HAVE_QUAD
extern char *llbtoa(char *buf, u_quad_t value, int base);
#endif
extern int atob(unsigned long *vp, const char *buf, int base);
#ifdef HAVE_QUAD
extern int llatob(u_quad_t *vp, const char *buf, int base);
#endif

#endif //__CONVERT_H__
