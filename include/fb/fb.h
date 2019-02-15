
/**
 * Copyright(c) 2016-12-18 Shangwen Wu	
 *
 * FB相关定义
 * 
 */

#ifndef __FB_H__
#define __FB_H__

/* 显示色深模式 */
#define FB_GDF_RGB332					0
#define FB_GDF_RGB555					1
#define FB_GDF_RGB565					2
#define FB_GDF_RGB888					3
#define FB_GDF_RGBX888					4
#define FB_GDF_1BIT						5
#define FB_GDF_2BIT						6
#define FB_GDF_4BIT						7
#define FB_GDF_8BIT_PALETTE				8			//暂时未实现

/* 方便访问graphic_device结构成员的宏定义 */
#define VIDEO_VISUAL_XSIZE				(gradevice.vsize_x)		//横坐标像素个数
#define VIDEO_VISUAL_YSIZE				(gradevice.vsize_y)		//纵坐标像素个数
#define VIDEO_MEM_BASE					(gradevice.vbase)		//显存基地址
#define VIDEO_BITS_PER_PIX				(gradevice.gdf_bpp)		//每个像素点占用位宽	
#define VIDEO_GDF_TYPE					(gradevice.gdf_type)	//色深模式
#define VIDEO_MEM_SIZE					(VIDEO_VISUAL_XSIZE*VIDEO_VISUAL_YSIZE*VIDEO_BITS_PER_PIX/8)		//显存总的byte数
#define VIDEO_LINE_SIZE					(VIDEO_VISUAL_XSIZE*VIDEO_BITS_PER_PIX/8)

struct graphic_device {
	unsigned char gdf_bpp;							//每个像素占用BIT数
	unsigned char gdf_type;							//图像数据格式
	unsigned int vsize_x, vsize_y;					//可视的水平，垂直像素个数
	unsigned long vbase;							//帧缓冲区首地址
};

extern void fb_fill(unsigned int color);
extern int fb_drawfont(unsigned char font, unsigned int x, unsigned int y, 
				unsigned int bgx, unsigned int fgx,
				unsigned int bgx1, unsigned int fgx1,
				unsigned int bgx2, unsigned int fgx2);
extern int fb_drawfont_reverse(unsigned int x, unsigned int y, 
				unsigned int bgx, unsigned int fgx,
				unsigned int bgx1, unsigned int fgx1,
				unsigned int bgx2, unsigned int fgx2);
extern void fb_move(unsigned int dst_pix, unsigned int src_pix, unsigned int n);
extern void fb_set(unsigned int dst_pix, unsigned int n, unsigned int color);

#endif /* __FB_H__ */

