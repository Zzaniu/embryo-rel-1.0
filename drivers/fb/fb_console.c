
/**
 * Copyright(c) 2016-12-20 Shangwen Wu	
 *
 * FB驱动相关程序
 * 
 */
#include <mach/types.h>
#include <stdlib.h>
#include <fs/termio.h>
#include <asm/io.h>
#include "font8x16.h" 
#include <fb/fb.h>
#include <fb/fb_console.h>

/* 软件模拟光标 */
#define cursor_set()	fb_drawfont_reverse(fcinfo.console_col * VIDEO_FONT_WIDTH,	\
							fcinfo.console_raw * VIDEO_FONT_HEIGHT, 			\
							fcinfo.bgx, fcinfo.fgx, 							\
							fcinfo.bgx1, fcinfo.bgx1, 							\
							fcinfo.bgx2, fcinfo.bgx2);

#define cursor_off()	fb_drawfont_reverse(fcinfo.console_col * VIDEO_FONT_WIDTH,	\
							fcinfo.console_raw * VIDEO_FONT_HEIGHT, 			\
							fcinfo.bgx, fcinfo.fgx, 							\
							fcinfo.bgx1, fcinfo.bgx1, 							\
							fcinfo.bgx2, fcinfo.bgx2);

extern int vga_available; 			//defined in machdep.c
static struct fbcon_info fcinfo;

/**
 * 描述：显示控制台初始化
 * 参数：bg为指定色深下的RGB背景色，fg为指定色深下的前景色；
 * 		 raw_max/col_max为控制台允许最大像素宽度
 *
*/
int fbconsole_init(unsigned int col_max, unsigned int raw_max,
					unsigned int bg, unsigned int fg, unsigned char bpp)
{
	switch (bpp) {	
	case 1:
		fcinfo.bgx = bg ? 0xff : 0x00;
		fcinfo.fgx = fg ? 0xff : 0x00;
	break;
	case 2:
		fcinfo.bgx = bg ? 0xff : 0x00;
		fcinfo.fgx = fg ? 0xff : 0x00;
	break;
	case 4:
		fcinfo.bgx = bg ? 0xffff : 0x00;
		fcinfo.fgx = fg ? 0xffff : 0x00;
	break;
	case 8:
		fcinfo.bgx = (bg & 0xff) | ((bg & 0xff) << 8) | ((bg & 0xff) << 16) | ((bg & 0xff) << 24);
		fcinfo.fgx = (fg & 0xff) | ((fg & 0xff) << 8) | ((fg & 0xff) << 16) | ((fg & 0xff) << 24);
	break;
	case 15:
		fcinfo.bgx = (bg & 0x7fff) | ((bg & 0x7fff) << 16);
		fcinfo.fgx = (fg & 0x7fff) | ((fg & 0x7fff) << 16);
	break;
	case 16:
		fcinfo.bgx = (bg & 0xffff) | ((bg & 0xffff) << 16);
		fcinfo.fgx = (fg & 0xffff) | ((fg & 0xffff) << 16);
	break;
	case 24:
		fcinfo.bgx = (bg & 0xffffff) | ((bg & 0xff) << 24);
		fcinfo.bgx1 = ((bg >> 8) & 0xffff) | ((bg << 16) & 0xffff0000);
		fcinfo.bgx2 = ((bg >> 24) & 0xff) | ((bg << 8) & 0xffffff00);
		fcinfo.fgx = (fg & 0xffffff) | ((fg & 0xff) << 24);
		fcinfo.fgx1 = ((fg >> 8) & 0xffff) | ((fg << 16) & 0xffff0000);
		fcinfo.fgx2 = ((fg >> 24) & 0xff) | ((fg << 8) & 0xffffff00);
	break;
	case 32:
		fcinfo.bgx = (bg & 0xffffff);
		fcinfo.fgx = (fg & 0xffffff);
	break;
	default:
		printf("fb console init failed, unsupported GDF!\n");
		return -1;
	}	
	
	fcinfo.bg = bg;
	fcinfo.fg = fg;
	fcinfo.console_raw = 0;
	fcinfo.console_col = 0;
	fcinfo.raw_limit = raw_max / VIDEO_FONT_HEIGHT;
	fcinfo.col_limit = col_max / VIDEO_FONT_WIDTH;
	fcinfo.console_xsize = col_max;
	fcinfo.console_ysize = raw_max;

#ifdef GRAPHIC_ACCELERATE_SW
	if(NULL == (fcinfo.matrix_map = malloc(fcinfo.raw_limit * fcinfo.col_limit))) {
		printf("fb console init failed, out of memory!\n");
		return -1;
	}
	memset32(fcinfo.matrix_map, 0, fcinfo.raw_limit * fcinfo.col_limit >> 2);		//clr
#endif

	fb_fill(bg);						//将整个屏幕填充背景色

	return 0;
}

static int fbcon_init(struct tty_device *dev)
{
	return 0;
}

static int fbcon_open(struct tty_device *dev)
{
	return 0;				
}

static int fbcon_reset(struct tty_device *dev)
{
	return 0;
}

static int fbcon_setbaud(struct tty_device *dev, int baud)
{
	return 0;
}

static int fbcon_tx_ready(struct tty_device *dev)
{
	return 1;
}

static void fbcon_scrollup(void)
{
#ifdef GRAPHIC_ACCELERATE_SW
	unsigned int x, y, pos;
	unsigned int col = fcinfo.col_limit;
	unsigned int raw = fcinfo.raw_limit;
	unsigned char *map = fcinfo.matrix_map;

	for(y = 1; y < raw; ++y) {
		pos = y * col;
		for(x = 0; x < col; ++x) {
			if(map[pos + x] != map[pos - col + x])
				fb_drawfont(map[pos + x], x * VIDEO_FONT_WIDTH, 
							(y - 1) * VIDEO_FONT_HEIGHT, 
							fcinfo.bgx, fcinfo.fgx, 
							fcinfo.bgx1, fcinfo.fgx1, 
							fcinfo.bgx2, fcinfo.fgx2);
		}
	}
	memcpy32(map, map + col, ((raw - 1) * col) >> 2);
	//清空最后一行对应位置
	memset32(map + (raw - 1) * col, 0, col >> 2);
#else
	//将整个屏幕整个向上移动一行
	fb_move(0, fcinfo.console_xsize * VIDEO_FONT_HEIGHT, 
			(fcinfo.raw_limit - 1) * fcinfo.console_xsize * VIDEO_FONT_HEIGHT);
#endif

	//清空最后一行
	fb_set((fcinfo.raw_limit - 1) * fcinfo.console_xsize * VIDEO_FONT_HEIGHT, 
			fcinfo.console_xsize * VIDEO_FONT_HEIGHT, fcinfo.bg);
}

static void fbcon_newline(void)
{
	fcinfo.console_col = 0;
	++fcinfo.console_raw;

	if(fcinfo.console_raw >= fcinfo.raw_limit) {
		//滚动控制台
		fbcon_scrollup();
		--fcinfo.console_raw;
	}
}

static void fbcon_backspace(void)
{
	--fcinfo.console_col;

	if(fcinfo.console_col < 0) {
		fcinfo.console_col = 0;
	}
}

static int fbcon_putchar(unsigned char c)
{
	switch (c) {
	case '\r':								//忽略\r
		cursor_off();
	break;
	case '\n':
		cursor_off();
		fbcon_newline();
	break;
	case 8:									//回退
		cursor_off();
		fbcon_backspace();	
	break;
	default:
		fb_drawfont(c, fcinfo.console_col * VIDEO_FONT_WIDTH, 
					fcinfo.console_raw * VIDEO_FONT_HEIGHT, 
					fcinfo.bgx, fcinfo.fgx, 
					fcinfo.bgx1, fcinfo.fgx1, 
					fcinfo.bgx2, fcinfo.fgx2);
#ifdef GRAPHIC_ACCELERATE_SW
		fcinfo.matrix_map[fcinfo.console_raw * fcinfo.col_limit + fcinfo.console_col] = c; 
#endif		
		++fcinfo.console_col;
		if(fcinfo.console_col >= fcinfo.col_limit) 
			fbcon_newline();
	}
	cursor_set();
}

static int fbcon_tx_byte(struct tty_device *dev, unsigned char data)
{
	return (vga_available ? fbcon_putchar(data) : 0);
}

static int fbcon_rx_ready(struct tty_device *dev)
{
	return 0;
}

static int fbcon_rx_byte(struct tty_device *dev, unsigned char *data)
{
	return 0;
}

static int fbcon_rx_hardflwctl(struct tty_device *dev, int rx_stop)
{
	return 0;
}

static int fbcon_tx_hardflwctl(struct tty_device *dev, int *tx_ready)
{
	return 0;
}

/* 标准IO的终端接口函数定义 */
struct tty_operations fbcon_ops = {
	.tty_device_init = fbcon_init,
	.tty_device_open = fbcon_open,
	.tty_device_reset = fbcon_reset,
	.tty_set_baudrate = fbcon_setbaud,
	.tty_tx_ready = fbcon_tx_ready,
	.tty_tx_byte = fbcon_tx_byte,
	.tty_rx_ready = fbcon_rx_ready,
	.tty_rx_byte = fbcon_rx_byte,
	.tty_rx_hardflwctl = fbcon_rx_hardflwctl,
	.tty_tx_hardflwctl = fbcon_tx_hardflwctl,
};
