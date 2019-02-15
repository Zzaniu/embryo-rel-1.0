
/**
 * Copyright(c) 2016-12-20 Shangwen Wu	
 *
 * FB控制台相关定义
 * 
 */

#ifndef __FB_CONSOLE_H__
#define __FB_CONSOLE_H__

#define GRAPHIC_ACCELERATE_SW								/* 软件加速显示控制台处理 */

/* 显示控制台信息结构 */
struct fbcon_info {
	unsigned int bg, fg;									//前景/背景色
	unsigned int bgx, fgx;									//前景/背景色四字节掩码
	unsigned int bgx1, fgx1;								//专门为RGB888用的前景/背景色掩码
	unsigned int bgx2, fgx2;								//专门为RGB888用的前景/背景色掩码
	int console_raw, console_col;							//当前字体坐标
	int raw_limit, col_limit;								//行列最大字体个数
	unsigned int console_xsize, console_ysize;				//控制台宽高像素数
#ifdef GRAPHIC_ACCELERATE_SW
	unsigned char *matrix_map;								//控制台矩阵位图，用于加速控制台滚动处理
#endif
};

#endif /* __FB_CONSOLE_H__ */

