
/**
 * Copyright(c) 2017-5-7 Shangwen Wu	
 *
 * 系统同步相关头文件
 * 
 */

#ifndef __SYS_SYNC_H__
#define __SYS_SYNC_H__

//defined in kern_sync.c
extern int tsleep(void);	
extern void wakeup(void);		

#endif //__SYS_SYNC_H__
