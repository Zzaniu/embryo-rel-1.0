
/**
 * Copyright(c) 2016-12-30 Shangwen Wu	
 *
 * 系统日志信息相关头文件
 * 
 */

#ifndef __SYS_SYSLOG_H__
#define __SYS_SYSLOG_H__

/* 系统级别定义 */
#define LOG_EMERG			0			/* 系统毁灭性错误 */
#define LOG_ALERT			1			/* 当前行为必须立即停止 */
#define LOG_CRIT			2			/* 致命错误 */
#define LOG_ERR				3			/* 一般错误 */
#define LOG_WARNING			4			/* 警告 */
#define LOG_NOTICE			5			/* 提示信息 */
#define LOG_INFO			6			/* 一般信息 */
#define LOG_DEBUG			7			/* 调试 */

#define LOG_LVLMASK			0x07		/* 日志级别掩码 */
#define LOG_LVL(l)			((l) & LOG_LVLMASK)

extern void loginit(void);

extern int log(int lvl, const char *fmt, ...);

#endif //__SYS_SYSLOG_H__

