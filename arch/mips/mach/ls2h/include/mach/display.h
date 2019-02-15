
/**
 * Copyright(c) 2016-12-17 Shangwen Wu	
 *
 * 显示配置相关头文件
 * 
 */
#ifndef __MACH_DISPLAY_H__
#define __MACH_DISPLAY_H__

#include <autoconf.h>

#if VIDEO_640X480
	#define FB_XSIZE			640
	#define FB_YSIZE			480
#elif VIDEO_640X640		
	#define FB_XSIZE			640	
	#define FB_YSIZE			640
#elif VIDEO_640X768		
	#define FB_XSIZE			640	
	#define FB_YSIZE			768
#elif VIDEO_640X800		
	#define FB_XSIZE			640	
	#define FB_YSIZE			800
#elif VIDEO_800X480		
	#define FB_XSIZE			800
	#define FB_YSIZE			480
#elif VIDEO_800X600		
	#define FB_XSIZE			800	
	#define FB_YSIZE			600
#elif VIDEO_800X640		
	#define FB_XSIZE			800	
	#define FB_YSIZE			640
#elif VIDEO_832X600		
	#define FB_XSIZE			832
	#define FB_YSIZE			600
#elif VIDEO_832X608		
	#define FB_XSIZE			832
	#define FB_YSIZE			608
#elif VIDEO_1024X480
	#define FB_XSIZE			1024
	#define FB_YSIZE			480
#elif VIDEO_1024X600
	#define FB_XSIZE			1024
	#define FB_YSIZE			600
#elif VIDEO_1024X640
	#define FB_XSIZE			1024
	#define FB_YSIZE			640
#elif VIDEO_1024X768
	#define FB_XSIZE			1024
	#define FB_YSIZE			768
#elif VIDEO_1152X764
	#define FB_XSIZE			1152
	#define FB_YSIZE			764
#elif VIDEO_1280X800
	#define FB_XSIZE			1280
	#define FB_YSIZE			800
#elif VIDEO_1280X1024
	#define FB_XSIZE			1280
	#define FB_YSIZE			1024
#elif VIDEO_1440X800
	#define FB_XSIZE			1440
	#define FB_YSIZE			800
#elif VIDEO_1440X900
	#define FB_XSIZE			1440
	#define FB_YSIZE			900
#else
	#define FB_XSIZE			800	
	#define FB_YSIZE			600
#endif

#endif //__MACH_DISPLAY_H__
