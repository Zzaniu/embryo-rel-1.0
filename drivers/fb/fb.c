
/**
 * Copyright(c) 2016-12-18 Shangwen Wu	
 *
 * FB驱动相关程序
 * 
 */
#include <mach/types.h>
#include <string.h>
#include <stdio.h>
#include <fs/termio.h>
#include <asm/io.h>
#include <fb/fb.h>
#include "font8x16.h" 

#ifdef VIDEO_FONT_LITTLE_ENDIAN							//defined in font8x16.h
	#define SWAP16(x)			((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
	#define SHORTSWAP32(x)		((((x) & 0xffff) << 16) | (((x) & 0xffff0000) >> 16))
	#define SWAP32(x)			((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | \
								(((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))
#else
	#define SWAP16(x)			(x)
	#define SHORTSWAP32(x)		(x)
	#define SWAP32(x)			(x)
#endif

static struct graphic_device gradevice;					//描述当前显示设备的像素信息

/* 对应于字体一行像素在各种色深下的位图 */
static u8 font_bitmap_tab2[16] = {		/* 用于2BIT模式，每个元素表示4个像素 */
	0x00, 								/* 0000:b'00000000 */
	0x03,	 							/* 0001:b'00000011 */
	0x0c, 								/* 0010:b'00001100 */
	0x0f, 								/* 0011:b'00001111 */
	0x30,								/* 0100:b;00110000 */
	0x33,								/* 0101:b;00110011 */
	0x3c,								/* 0110:b;00111100 */
	0x3f,								/* 0111:b;00111111 */
	0xc0,								/* 1000:b;11000000 */
	0xc3,								/* 1001:b;11000011 */
	0xcc,								/* 1010:b;11001100 */
	0xcf,								/* 1011:b;11001111 */
	0xf0,								/* 1100:b;11110000 */
	0xf3,								/* 1101:b;11110011 */
	0xfc,								/* 1110:b;11111100 */
	0xff,								/* 1111:b;11111111 */
};

static u16 font_bitmap_tab4[16] = {		/* 用于4BIT模式，每个元素表示4个像素 */
	0x0000, 							/* 0000:0x0000 */
	0x000f,	 							/* 0001:0x000f */
	0x00f0, 							/* 0010:0x00f0 */
	0x00ff, 							/* 0011:0x00ff */
	0x0f00,								/* 0100:0x0f00 */
	0x0f0f,								/* 0101:0x0f0f */
	0x0ff0,								/* 0110:0x0ff0 */
	0x0fff,								/* 0111:0x0fff */
	0xf000,								/* 1000:0xf000 */
	0xf00f,								/* 1001:0xf00f */
	0xf0f0,								/* 1010:0xf0f0 */
	0xf0ff,								/* 1011:0xf0ff */
	0xff00,								/* 1100:0xff00 */
	0xff0f,								/* 1101:0xff0f */
	0xfff0,								/* 1110:0xfff0 */
	0xffff,								/* 1111:0xffff */
};

static u32 font_bitmap_tab8[16] = {		/* 用于RGB332，每个元素表示4个像素 */
	0x00000000, 						/* 0000:0x00000000 */
	0x000000ff,	 						/* 0001:0x000000ff */
	0x0000ff00, 						/* 0010:0x0000ff00 */
	0x0000ffff, 						/* 0011:0x0000ffff */
	0x00ff0000,							/* 0100:0x00ff0000 */
	0x00ff00ff,							/* 0101:0x00ff00ff */
	0x00ffff00,							/* 0110:0x00ffff00 */
	0x00ffffff,							/* 0111:0x00ffffff */
	0xff000000,							/* 1000:0xff000000 */
	0xff0000ff,							/* 1001:0xff0000ff */
	0xff00ff00,							/* 1010:0xff00ff00 */
	0xff00ffff,							/* 1011:0xff00ffff */
	0xffff0000,							/* 1100:0xffff0000 */
	0xffff00ff,							/* 1101:0xffff00ff */
	0xffffff00,							/* 1110:0xffffff00 */
	0xffffffff,							/* 1111:0xffffffff */
};

static u32 font_bitmap_tab15[4] = {		/* 用于RGB555模式，每个元素表示2个像素 */
	0x00000000, 						/* 00:0x00000000 */
	0x00007fff,	 						/* 01:0x00007fff */
	0x7fff0000, 						/* 10:0x7fff0000 */
	0x7fff7fff, 						/* 11:0x7fff7fff */
};

static u32 font_bitmap_tab16[4] = {		/* 用于RGB565模式，每个元素表示2个像素 */
	0x00000000, 						/* 00:0x00000000 */
	0x0000ffff,	 						/* 01:0x0000ffff */
	0xffff0000, 						/* 10:0xffff0000 */
	0xffffffff, 						/* 11:0xffffffff */
};

static u32 font_bitmap_tab24[16][3] = {	/* 用于RGB888模式，每一组元素表示4个像素 */
	{0x00000000, 0x00000000, 0x00000000}, 	/* 0000 */
	{0x00000000, 0x00000000, 0x00ffffff}, 	/* 0001 */
	{0x00000000, 0x0000ffff, 0xff000000}, 	/* 0010 */
	{0x00000000, 0x0000ffff, 0xffffffff}, 	/* 0011 */
	{0x000000ff, 0xffff0000, 0x00000000}, 	/* 0100 */
	{0x000000ff, 0xffff0000, 0x00ffffff}, 	/* 0101 */
	{0x000000ff, 0xffffffff, 0xff000000}, 	/* 0110 */
	{0x000000ff, 0xffffffff, 0xffffffff}, 	/* 0111 */
	{0xffffff00, 0x00000000, 0x00000000}, 	/* 1000 */
	{0xffffff00, 0x00000000, 0x00ffffff}, 	/* 1001 */
	{0xffffff00, 0x0000ffff, 0xff000000}, 	/* 1010 */
	{0xffffff00, 0x0000ffff, 0xffffffff}, 	/* 1011 */
	{0xffffffff, 0xffff0000, 0x00000000}, 	/* 1100 */
	{0xffffffff, 0xffff0000, 0x00ffffff}, 	/* 1101 */
	{0xffffffff, 0xffffffff, 0xff000000}, 	/* 1110 */
	{0xffffffff, 0xffffffff, 0xffffffff}, 	/* 1111 */
};

static u32 font_bitmap_tab32[16][4] = {	/* 用于RGBX888模式，每一组元素表示4个像素 */
	{0x00000000, 0x00000000, 0x00000000, 0x00000000}, 	/* 0000 */
	{0x00000000, 0x00000000, 0x00000000, 0x00ffffff}, 	/* 0001 */
	{0x00000000, 0x00000000, 0x00ffffff, 0x00000000}, 	/* 0010 */
	{0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff}, 	/* 0011 */
	{0x00000000, 0x00ffffff, 0x00000000, 0x00000000}, 	/* 0100 */
	{0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff}, 	/* 0101 */
	{0x00000000, 0x00ffffff, 0x00ffffff, 0x00000000}, 	/* 0110 */
	{0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff}, 	/* 0111 */
	{0x00ffffff, 0x00000000, 0x00000000, 0x00000000}, 	/* 1000 */
	{0x00ffffff, 0x00000000, 0x00000000, 0x00ffffff}, 	/* 1001 */
	{0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000}, 	/* 1010 */
	{0x00ffffff, 0x00000000, 0x00ffffff, 0x00ffffff}, 	/* 1011 */
	{0x00ffffff, 0x00ffffff, 0x00000000, 0x00000000}, 	/* 1100 */
	{0x00ffffff, 0x00ffffff, 0x00000000, 0x00ffffff}, 	/* 1101 */
	{0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00000000}, 	/* 1110 */
	{0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff}, 	/* 1111 */
};

/**
 * 描述：FB驱动初始化
 * 参数：membase，显存基地址；xsize，水平像素个数，ysize，垂直像素个数，bpp，每个像素占多少bit
*/
int fb_init(unsigned long vmembase, unsigned int xsize, unsigned int ysize, unsigned char bpp)
{
	gradevice.vbase = vmembase;
	gradevice.vsize_x = xsize;
	gradevice.vsize_y = ysize;
	
	switch (bpp) {	
	case 1:
		gradevice.gdf_type = FB_GDF_1BIT;	
	break;
	case 2:
		gradevice.gdf_type = FB_GDF_2BIT;	
	break;
	case 4:
		gradevice.gdf_type = FB_GDF_4BIT;	
	break;
	case 8:
		gradevice.gdf_type = FB_GDF_RGB332;	
	break;
	case 15:
		gradevice.gdf_type = FB_GDF_RGB555;
	break;
	case 16:
		gradevice.gdf_type = FB_GDF_RGB565;
	break;
	case 24:
		gradevice.gdf_type = FB_GDF_RGB888;
	break;
	case 32:
		gradevice.gdf_type = FB_GDF_RGBX888;
	break;
	default:
		printf("fb init failed, unsupported GDF!\n");
		return -1;
	}	
	gradevice.gdf_bpp = bpp == 15 ? ++bpp : bpp;

	return 0;
}

/**
 * 描述：填充整个FB为指定色深的颜色值
 * 参数：color为指定色深下的颜色值
 */
void fb_fill(unsigned int color)
{
	unsigned int i;
	u8 *dst = (u8 *)gradevice.vbase;

	switch(VIDEO_GDF_TYPE) {
	case FB_GDF_1BIT:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 4) 
			*(u32 *)(dst + i) = (color & 0x01) ? 0xffffffff: 0x00; 
	break;
	case FB_GDF_2BIT:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 4)
			*(u32 *)(dst + i) = (color & 0x03) ? 0xffffffff: 0x00; 
	break;
	case FB_GDF_4BIT:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 4)
			*(u32 *)(dst + i) = (color & 0x0f) ? 0xffffffff: 0x00; 
	break;
	case FB_GDF_RGB332:
		for(i = 0 ; i < VIDEO_MEM_SIZE; ++i)
			*(dst + i) = color & 0xff;
	break;
	case FB_GDF_RGB555:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 2) 
			*(u16 *)(dst + i) = color & 0x7fff;
	break;
	case FB_GDF_RGB565:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 2) 
			*(u16 *)(dst + i) = color & 0xffff;
	break;
	case FB_GDF_RGB888:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 3) { 
			*(dst + i) = color & 0xff;
			*(dst + i + 1) = (color >> 8) & 0xff;
			*(dst + i + 2) = (color >> 16) & 0xff;
		}
	break;
	case FB_GDF_RGBX888:
		for(i = 0 ; i < VIDEO_MEM_SIZE; i += 4) 
			*(u32 *)(dst + i) = color & 0x00ffffff;
	break;
	}
}

/**
 * 描述：绘制字符
 * 参数：font，要描绘的字符；x/y，字母左上角第一个像素点的坐标；bgx/fgx，背景和前景颜色掩码；
 * 		 base code：参数设计得不好
 * 返回：成功描绘的字母个数
 */
int fb_drawfont(unsigned char font, unsigned int x, unsigned int y, 
				unsigned int bgx, unsigned int fgx,
				unsigned int bgx1, unsigned int fgx1,
				unsigned int bgx2, unsigned int fgx2)
{	
	int raw;
	unsigned int offset, eorx, eorx1, eorx2;
	unsigned char *cdat, bits;
	u32 *dest32;
	u16 *dest16;
	u8 *dest8;

	eorx = bgx ^ fgx;
	eorx1 = bgx1 ^ fgx1;
	eorx2 = bgx2 ^ fgx2;
	cdat = video_fontdata + font * VIDEO_FONT_HEIGHT; 
	offset = VIDEO_LINE_SIZE * y + x * VIDEO_BITS_PER_PIX / 8;
	dest32 = (u32 *)(VIDEO_MEM_BASE + offset);
	dest16 = (u16 *)(VIDEO_MEM_BASE + offset);
	dest8 = (u8 *)(VIDEO_MEM_BASE + offset);

	//printf("draw font: %c, x=%u, y=%u, bgx=0x%x, fgx=0x%x, eorx=0x%x, offset=%u, type=%u, line=%u, bpp=%u\n", 
	//		font, x, y, bgx, fgx, eorx, offset, VIDEO_GDF_TYPE, VIDEO_LINE_SIZE, VIDEO_BITS_PER_PIX);

	switch(VIDEO_GDF_TYPE) {
	case FB_GDF_1BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest8 + raw * VIDEO_LINE_SIZE) = (bits & eorx) ^ bgx;
		}
	break;	
	case FB_GDF_2BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest8 + raw * VIDEO_LINE_SIZE) = (font_bitmap_tab2[bits >> 4] & eorx) ^ bgx;
			*(dest8 + raw * VIDEO_LINE_SIZE + 1) = (font_bitmap_tab2[bits & 0x0f] & eorx) ^ bgx;
		}
	break;	
	case FB_GDF_4BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest16 + (raw * VIDEO_LINE_SIZE >> 1)) = SWAP16((font_bitmap_tab4[bits >> 4] & eorx) ^ bgx);
			*(dest16 + (raw * VIDEO_LINE_SIZE >> 1) + 1) = SWAP16((font_bitmap_tab4[bits & 0x0f] & eorx) ^ bgx);
		}
	break;	
	case FB_GDF_RGB332:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) = SWAP32((font_bitmap_tab8[bits >> 4] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) = SWAP32((font_bitmap_tab8[bits & 0x0f] & eorx) ^ bgx);
		}
	break;	
	case FB_GDF_RGB555:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) = SHORTSWAP32((font_bitmap_tab15[bits >> 6] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) = SHORTSWAP32((font_bitmap_tab15[(bits >> 4) & 0x03] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) = SHORTSWAP32((font_bitmap_tab15[(bits >> 2 ) & 0x03] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) = SHORTSWAP32((font_bitmap_tab15[bits & 0x03] & eorx) ^ bgx);
		}
	break;	
	case FB_GDF_RGB565:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) = SHORTSWAP32((font_bitmap_tab16[bits >> 6] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) = SHORTSWAP32((font_bitmap_tab16[(bits >> 4) & 0x03] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) = SHORTSWAP32((font_bitmap_tab16[(bits >> 2 ) & 0x03] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) = SHORTSWAP32((font_bitmap_tab16[bits & 0x03] & eorx) ^ bgx);
		}
	break;	
	case FB_GDF_RGB888:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) = SWAP32((font_bitmap_tab24[bits >> 4][0] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) = SWAP32((font_bitmap_tab24[bits >> 4][1] & eorx1) ^ bgx1);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) = SWAP32((font_bitmap_tab24[bits >> 4][2] & eorx2) ^ bgx2);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) = SWAP32((font_bitmap_tab24[bits & 0x0f][0] & eorx) ^ bgx);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 4) = SWAP32((font_bitmap_tab24[bits & 0x0f][1] & eorx1) ^ bgx1);
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 5) = SWAP32((font_bitmap_tab24[bits & 0x0f][2] & eorx2) ^ bgx2);
		}
	break;	
	case FB_GDF_RGBX888:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			bits = cdat[raw];
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) = (font_bitmap_tab32[bits >> 4][0] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) = (font_bitmap_tab32[bits >> 4][1] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) = (font_bitmap_tab32[bits >> 4][2] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) = (font_bitmap_tab32[bits >> 4][3] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 4) = (font_bitmap_tab32[bits & 0xf][0] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 5) = (font_bitmap_tab32[bits & 0xf][1] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 6) = (font_bitmap_tab32[bits & 0xf][2] & eorx) ^ bgx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 7) = (font_bitmap_tab32[bits & 0xf][3] & eorx) ^ bgx;
		}
	break;	
	default:
		return -1;
	}

	return 0;
}

/**
 * 描述：缓冲区搬移操作，该函数实现屏幕滚动效率过于地下
 * 参数：dst_pix表示移动的目标像素点偏移，src_pix表示要移动的像素点位置，n为移动像素点个数
 */
void fb_move(unsigned int dst_pix, unsigned int src_pix, unsigned int n)
{
	u32 *dst = (u32 *)(VIDEO_MEM_BASE + dst_pix * VIDEO_BITS_PER_PIX / 8);
	u32 *src = (u32 *)(VIDEO_MEM_BASE + src_pix * VIDEO_BITS_PER_PIX / 8);
	unsigned int nint = n * VIDEO_BITS_PER_PIX / 8 / 4;

	memcpy32(dst, src, nint);	
}

/**
 * 描述：设置指定的FB区域为指定色深的颜色值
 * 参数：dst_pix表示移动的目标像素点偏移，n为设置的像素点个数，color为指定色深下的颜色值
 */
void fb_set(unsigned int dst_pix, unsigned int n, unsigned int color)
{
	unsigned int i;
	u8 *dst = (u8 *)gradevice.vbase + dst_pix * VIDEO_BITS_PER_PIX / 8;

	if(!n)
		return;
	if(n + dst_pix >= VIDEO_VISUAL_XSIZE * VIDEO_VISUAL_YSIZE)
		n = VIDEO_VISUAL_XSIZE * VIDEO_VISUAL_YSIZE - 1 - dst_pix;
        
	switch(VIDEO_GDF_TYPE) {
	case FB_GDF_1BIT:									//bad codes 暂不支持对非字节对称像素点的支持
		if((dst_pix & 0x7) || (n & 0x7)) 
			return;
		for(i = 0 ; i < n / 8; ++i) 
			*(dst + i) = (color & 0x01) ? 0xff: 0x00; 
	break;
	case FB_GDF_2BIT:
		if((dst_pix & 0x3) || (n & 0x3)) 
			return;
		for(i = 0 ; i < n / 4; ++i)
			*(dst + i) = (color & 0x03) ? 0xff: 0x00; 
	break;
	case FB_GDF_4BIT:
		if((dst_pix & 0x1) || (n & 0x1)) 
			return;
		for(i = 0 ; i < n / 2; ++i)
			*(dst + i) = (color & 0x0f) ? 0xff: 0x00; 
	break;
	case FB_GDF_RGB332:
		for(i = 0 ; i < n; ++i)
			*(dst + i) = color & 0xff;
	break;
	case FB_GDF_RGB555:
		for(i = 0 ; i < n; ++i) 
			*((u16 *)dst + i) = color & 0x7fff;
	break;
	case FB_GDF_RGB565:
		for(i = 0 ; i < n; ++i) 
			*((u16 *)dst + i) = color & 0xffff;
	break;
	case FB_GDF_RGB888:
		for(i = 0 ; i < n; ++i) { 
			*(dst + i * 3 ) = color & 0xff;
			*(dst + i * 3 + 1) = (color >> 8) & 0xff;
			*(dst + i * 3 + 2) = (color >> 16) & 0xff;
		}
	break;
	case FB_GDF_RGBX888:
		for(i = 0 ; i < n; ++i) 
			*((u32 *)dst + i) = color & 0x00ffffff;
	break;
	}
}

/**
 * 描述：字体颜色反转
 * 参数：x/y，字母左上角第一个像素点的坐标；bgx/fgx，背景和前景颜色掩码；
 * 		 base code：参数设计得不好
 * 返回：成功描绘的字母个数
 */
int fb_drawfont_reverse(unsigned int x, unsigned int y, 
				unsigned int bgx, unsigned int fgx,
				unsigned int bgx1, unsigned int fgx1,
				unsigned int bgx2, unsigned int fgx2)
{	
	int raw;
	unsigned int offset, eorx, eorx1, eorx2;
	u32 *dest32;
	u16 *dest16;
	u8 *dest8;

	eorx = bgx ^ fgx;
	eorx1 = bgx1 ^ fgx1;
	eorx2 = bgx2 ^ fgx2;
	offset = VIDEO_LINE_SIZE * y + x * VIDEO_BITS_PER_PIX / 8;
	dest32 = (u32 *)(VIDEO_MEM_BASE + offset);
	dest16 = (u16 *)(VIDEO_MEM_BASE + offset);
	dest8 = (u8 *)(VIDEO_MEM_BASE + offset);

	//printf("x=%u, y=%u, bgx=0x%x, fgx=0x%x, eorx=0x%x, offset=%u, type=%u, line=%u, bpp=%u\n", 
	//		x, y, bgx, fgx, eorx, offset, VIDEO_GDF_TYPE, VIDEO_LINE_SIZE, VIDEO_BITS_PER_PIX);

	switch(VIDEO_GDF_TYPE) {
	case FB_GDF_1BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest8 + raw * VIDEO_LINE_SIZE) ^= eorx;
		}
	break;	
	case FB_GDF_2BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest8 + raw * VIDEO_LINE_SIZE) ^= eorx;
			*(dest8 + raw * VIDEO_LINE_SIZE + 1) ^= eorx;
		}
	break;	
	case FB_GDF_4BIT:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest16 + (raw * VIDEO_LINE_SIZE >> 1)) ^= eorx;
			*(dest16 + (raw * VIDEO_LINE_SIZE >> 1) + 1) ^= eorx;
		}
	break;	
	case FB_GDF_RGB332:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) ^= eorx;
		}
	break;	
	case FB_GDF_RGB555:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) ^= eorx;
		}
	break;	
	case FB_GDF_RGB565:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) ^= eorx;
		}
	break;	
	case FB_GDF_RGB888:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) ^= eorx1;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) ^= eorx2;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 4) ^= eorx1;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 5) ^= eorx2;
		}
	break;	
	case FB_GDF_RGBX888:
		for(raw = 0; raw < VIDEO_FONT_HEIGHT; ++raw) {
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2)) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 1) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 2) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 3) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 4) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 5) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 6) ^= eorx;
			*(dest32 + (raw * VIDEO_LINE_SIZE >> 2) + 7) ^= eorx;
		}
	break;	
	default:
		return -1;
	}

	return 0;
}

