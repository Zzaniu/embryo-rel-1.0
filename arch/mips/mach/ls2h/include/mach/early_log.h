
/**
 * Copyright(c) 2015-10-1 Shangwen Wu	
 *
 * 终端初始化前日志函数相关头文件
 * 
 */
#ifndef __MACH_EARLY_LOG_H__
#define __MACH_EARLY_LOG_H__

#define EARLY_LOG_LEVEL		6

#define early_emerg(fmt, args...) do {}while(0) 
#define early_crit(fmt, args...) do {}while(0) 
#define early_err(fmt, args...) do {}while(0) 
#define early_warnning(fmt, args...) do {}while(0) 
#define early_info(fmt, args...) do {}while(0) 
#define early_debug(fmt, args...) do {}while(0) 

#if EARLY_LOG_LEVEL > 0
	#undef	early_emerg
	#define early_emerg(fmt, args...) early_printf(fmt, ##args)
#endif
#if EARLY_LOG_LEVEL > 1
	#undef	early_crit
	#define early_crit(fmt, args...) early_printf(fmt, ##args)
#endif
#if EARLY_LOG_LEVEL > 2
	#undef	early_err
	#define early_err(fmt, args...) early_printf(fmt, ##args)
#endif
#if EARLY_LOG_LEVEL > 3
	#undef	early_warnning
	#define early_warnning(fmt, args...) early_printf(fmt, ##args)
#endif
#if EARLY_LOG_LEVEL > 4
	#undef	early_info
	#define early_info(fmt, args...) early_printf(fmt, ##args)
#endif
#if EARLY_LOG_LEVEL > 5
	#undef	early_debug
	#define early_debug(fmt, args...) early_printf(fmt, ##args)
#endif

extern unsigned char early_getchar(void);

extern unsigned char early_putchar(unsigned char c);

extern int early_putstr(const char *s);

extern int early_getstr(char *s);

extern int early_printf(const char *fmt, ...);

#endif //__MACH_EARLY_LOG_H__
