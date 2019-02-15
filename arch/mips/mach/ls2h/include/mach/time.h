
/**
 * Copyright(c) 2016-2-1 Shangwen Wu	
 *
 * 板级相关时间头文件
 * 
 */
#ifndef __MACH_TIME_H__
#define __MACH_TIME_H__

typedef struct time_struct {
	unsigned long year;
	unsigned long month;
	unsigned long day;
	unsigned long hour;
	unsigned long min;
	unsigned long sec;
	unsigned long misec;
}time_struct_t;

int mach_rtc_init(void);

int mach_get_current_time(struct time_struct *ptime);

#endif //__MACH_TIME_H__

