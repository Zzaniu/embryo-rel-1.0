
/**
 * Copyright(c) 2016-2-1 Shangwen Wu	
 *
 * 龙芯2H时间相关处理
 * 
 */
#include <mach/types.h>
#include <mach/ls2h_regs.h>
#include <asm/io.h>
#include <mach/time.h>

/**
 * 描述：RTC控制器初始化，获取的当前时间若为非法值，
 * 		 则将时间重置为2000-1-1 00：00：00
 */
int mach_rtc_init(void)
{
	unsigned int year, month, day, hour, min, sec, misec, val;

	/* 使能RTC、TOY、外部晶振 */
	outl(LS2H_RTC_CTRL_REG, RTC_CTRL_REN | RTC_CTRL_TEN | RTC_CTRL_EO);
	outl(LS2H_TOY_TRIM_REG, 0x0);
	outl(LS2H_RTC_TRIM_REG, 0x0);

	year = (inl(LS2H_TOY_READ1_REG) >> RTC_TOY_YEAR_SHIFT);
	val	= inl(LS2H_TOY_READ0_REG);
	month = (val >> RTC_TOY_MONTH_SHIFT) & RTC_TOY_MONTH_MASK;
	day = (val >> RTC_TOY_DAY_SHIFT) & RTC_TOY_DAY_MASK;
	hour = (val >> RTC_TOY_HOUR_SHIFT) & RTC_TOY_HOUR_MASK;
	min = (val >> RTC_TOY_MIN_SHIFT) & RTC_TOY_MIN_MASK;
	sec = (val >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK;
	misec = (val >> RTC_TOY_MISEC_SHIFT) & RTC_TOY_MISEC_MASK;
	
	if(year > RTC_TOY_YEAR_MASK || 
			month < 1 || month > 12 ||
			day < 1 || day  > 31 ||
			hour > 23 ||
			min > 59 ||
			sec > 59 ||
			misec > 9) { 
		/* 使用默认时间:2000-1-1 00:00:00:00 */
		outl(LS2H_TOY_WRITE1_REG, 2000);
		outl(LS2H_TOY_WRITE0_REG, (1 << RTC_TOY_MONTH_SHIFT) | (1 << RTC_TOY_DAY_SHIFT));
	}
	
	return 0;
}

/**
 * 描述：获取当前RTC时间
 */
int mach_get_current_time(struct time_struct *ptime)
{
	unsigned int year, month, day, hour, min, sec, misec, val;

	year = (inl(LS2H_TOY_READ1_REG) >> RTC_TOY_YEAR_SHIFT);
	val	= inl(LS2H_TOY_READ0_REG);
	month = (val >> RTC_TOY_MONTH_SHIFT) & RTC_TOY_MONTH_MASK;
	day = (val >> RTC_TOY_DAY_SHIFT) & RTC_TOY_DAY_MASK;
	hour = (val >> RTC_TOY_HOUR_SHIFT) & RTC_TOY_HOUR_MASK;
	min = (val >> RTC_TOY_MIN_SHIFT) & RTC_TOY_MIN_MASK;
	sec = (val >> RTC_TOY_SEC_SHIFT) & RTC_TOY_SEC_MASK;
	misec = (val >> RTC_TOY_MISEC_SHIFT) & RTC_TOY_MISEC_MASK;
	
	ptime->year = year;
	ptime->month = month;
	ptime->day = day;
	ptime->hour = hour;
	ptime->min = min;
	ptime->sec = sec;
	ptime->misec = misec;

	return 0;
}

