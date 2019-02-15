
/**
 * Copyright(c) 2016-12-20 Shangwen Wu	
 *
 * RGB数据格式相关定义
 * 
 */

#ifndef __RGB_H__
#define __RGB_H__

#define RGB332(r, g, b)				((((r) & 0x7) << 5) | (((g) & 0x7) << 2) | ((b) & 0x3))
#define RGB555(r, g, b)				((((r) & 0x1f) << 10) | (((g) & 0x1f) << 5) | ((b) & 0x1f))
#define RGB565(r, g, b)				((((r) & 0x1f) << 11) | (((g) & 0x3f) << 5) | ((b) & 0x1f))
#define RGB888(r, g, b)				((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define RGBX888(r, g, b)			((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))

#endif //__RGB_H__

