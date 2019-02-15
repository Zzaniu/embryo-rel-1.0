
/**
 * Copyright(c) 2016-12-20 Shangwen Wu	
 *
 * FB控制台相关定义
 * 
 */

#ifndef __FONT8x16_H__
#define __FONT8x16_H__

#define VIDEO_FONT_LITTLE_ENDIAN
#define VIDEO_FONT_CHARS	256
#define VIDEO_FONT_WIDTH	8
#define VIDEO_FONT_HEIGHT	16
#define VIDEO_FONT_SIZE		(VIDEO_FONT_CHARS * VIDEO_FONT_HEIGHT)
#define VIDEO_FONT_PIXSIZE	(VIDEO_FONT_WIDTH * VIDEO_FONT_HEIGHT)

extern unsigned char video_fontdata[];						//defined in video_font.c 8x16字体库 

#endif /* __FONT8x16_H__ */

