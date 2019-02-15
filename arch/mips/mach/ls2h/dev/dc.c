
/**
 * Copyright(c) 2016-12-17 Shangwen Wu 
 *
 * 龙芯2H显示控制器初始化操作
 * 
 */
#include <mach/types.h>
#include <common.h>
#include <stdio.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <mach/bonito.h>
#include <mach/ls2h_regs.h>
#include <mach/display.h>
#include <sys/syslog.h>

unsigned char fb_bbp;					//FB驱动将使用这些全局变量
unsigned long fb_base;
unsigned int fb_xsize, fb_ysize;

/* Copyright(c) loongson */
/* 代码将根据分辨率匹配来选择适当的时钟以及行列总像素数 */
static struct fbmode {
	float pclk;
	int hr, hss, hse, hfl;				//水平显示像素，水平同步起始像素数，水平同步结束像素数，水平总像素数
	int vr, vss, vse, vfl;				//垂直显示像素，垂直同步起始像素数，垂直同步结束像素数，垂直总像素数
} fbmodes[] = {
	{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/* "640x480_70.00" */	
	{	33.10,	640,	672,	736,	832,	640,	641,	644,	663,	},	/* "640x640_60.00" */	
	{	39.69,	640,	672,	736,	832,	768,	769,	772,	795,	},	/* "640x768_60.00" */	
	{	42.13,	640,	680,	744,	848,	800,	801,	804,	828,	},	/* "640x800_60.00" */	
	{	35.84,	800,	832,	912,	1024,	480,	481,	484,	500,	},	/* "800x480_70.00" */	
	{	38.22,	800,	832,	912,	1024,	600,	601,	604,	622,	},	/* "800x600_60.00" */	
	{	40.73,	800,	832,	912,	1024,	640,	641,	644,	663,	},	/* "800x640_60.00" */	
	{	40.01,	832,	864,	952,	1072,	600,	601,	604,	622,	},	/* "832x600_60.00" */	
	{	40.52,	832,	864,	952,	1072,	608,	609,	612,	630,	},	/* "832x608_60.00" */	
	{	38.17,	1024,	1048,	1152,	1280,	480,	481,	484,	497,	},	/* "1024x480_60.00" */	
	{	48.96,	1024,	1064,	1168,	1312,	600,	601,	604,	622,	},	/* "1024x600_60.00" */	
	{	52.83,	1024,	1072,	1176,	1328,	640,	641,	644,	663,	},	/* "1024x640_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/* "1024x768_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/* "1024x768_60.00" */	
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/* "1024x768_60.00" */	
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/* "1152x764_60.00" */	
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/* "1280x800_60.00" */	
	{	98.60,	1280,	1352,	1488,	1696,	1024,	1025,	1028,	1057,	},	/* "1280x1024_55.00" */	
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/* "1440x800_60.00" */	
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/* "1440x900_67.00" */
};

/* Copyright(c) loongson，根据显示时钟计算所需要的分频系数，计算公式实在没搞明白 */
static u32 calcfreq(float pclk)
{
	u32 out;
	u32 pstdiv, ldf, odf;												//待测试的系数
	float a, b, c, min = 100.0;					
	u32 inta, intb, intc;										
	u32 pstdiv_df, pll_ldf, pll_odf, prediv_df = 0, pll_idf = 5;		//需要调整的5个系数，其中两个值固定

	for(pstdiv = 1; pstdiv < 32; pstdiv++) {
		for(odf = 1; odf <= 8; odf *= 2) {
			a = pclk * pstdiv;
			b = a * odf;
			ldf = ((int)(b / (50 * odf))) * odf;
			intb = ldf * 50;
			inta = b / odf;
			c = b - intb;
			if(inta < 75 || inta > 1800) 
				continue;
			if(intb < 600 || intb > 1800)
				continue;
			if(ldf < 8 || ldf > 225)
				continue;
			if(c < min) {
				min = c;
				pstdiv_df = pstdiv;
				pll_odf = odf;
				pll_ldf = ldf;
			}
		}
	}
	pll_odf = pll_odf == 8 ? 3 : pll_odf == 4 ? 2 : pll_odf == 2 ? 1 : 0;   
	out = (pstdiv_df << 24) | (pll_ldf << 16) | (prediv_df << 8) | (pll_odf << 5) | (pll_idf << 2) | 0x01; //选择sbc_clk作为参考时钟
	
	log(LOG_DEBUG, "PIXCLK: ODF = %d, LDF = %d, IDF = 5, pstdiv = %d, prediv = 1\n", pll_odf, pll_ldf, pstdiv_df);
	return out;
}

int dc_init(void)
{
	int mode, n = 300;
	u32 pdiv;
#if USE_DC_VGA 
	u32 dcbase = LS2H_DC_VGA_REGS_BASE;
	u32 clkbase = LS2H_PIXCLK1_CTRL_REG_BASE;
#else
	u32 dcbase = LS2H_DC_DVO_REGS_BASE;
	u32 clkbase = LS2H_PIXCLK0_CTRL_REG_BASE;
#endif

	/* 分频系数设置 */
	for(mode = 0; mode < NR(fbmodes); ++mode) {
		if(FB_XSIZE == fbmodes[mode].hr && FB_YSIZE == fbmodes[mode].vr) {
			pdiv = calcfreq(fbmodes[mode].pclk);
			outl(clkbase + PIXCLK_CTRL1, 0x0);					//选择参考时钟旁路
			outl(clkbase + PIXCLK_CTRL0, pdiv | 0x80000080);	//先将输出分频器置高进入修改状态，并将pll关断
			while(n--)
				inl(LS2H_CHIP_SAMPLE0_REG);						//等待一段时间，官方说这段时间为10000ns
			outl(clkbase + PIXCLK_CTRL0, pdiv);					//使输出分频器设置修改生效，并打开pll
			while((inl(LS2H_CHIP_SAMPLE0_REG) & 0x00001800) != 0x00001800)
				;												//等待pll锁定完成
			outl(clkbase + PIXCLK_CTRL1, 0x01);					//选择pll输出
			break;
		}
	}
	if(mode >= NR(fbmodes)) {
		printf("display controller init failed, unsupported framebuffer resolution!\n");	
		return -1;
	}
	
	/* 提升显存DMA的优先级 */
	outb(LS2H_QOS_CFG6_REG + 6, 0x36);							//手册中找不到该寄存器定义

	/* 将另外一路不做控制台的显示口输出当前控制台的拷贝信息 */
#if USE_DC_VGA 
	outl(LS2H_DC_DVO_REGS_BASE + DC_FB_CONF, 0x00100200);
#else
	outl(LS2H_DC_VGA_REGS_BASE + DC_FB_CONF, 0x00100200);
#endif

	outl(dcbase + DC_FB_CONF, 0x0);								/* 先禁用当前显示端口 */
	outl(dcbase + DC_FB_HWADDR0, FB_BASE);						/* 将两个缓冲区地址都指向FB_BASE */
	outl(dcbase + DC_FB_HWADDR1, FB_BASE);
	outl(dcbase + DC_FB_ORIGIN, 0x0);							/* 显示区左边无其他多余字节 */
	outl(dcbase + DC_DITHER_CONF, 0x0);							/* 关闭颜色抖动 */
	outl(dcbase + DC_DITHER_TAB_LO, 0x0);						/* 清零颜色抖动查找表 */
	outl(dcbase + DC_DITHER_TAB_HI, 0x0);						/* 清零颜色抖颜色抖动 */
	outl(dcbase + DC_PAN_CONF, 0x80000301);						/* 面板配置，该寄存器与手册中描述有差别，数据使能和数据使能极性值必须相反，位5表示数据极性取反，当位31置位时，必须保证时钟使能，而位31位为0时，所有bit为0或仅保证最低两bit相反即可显示 */
	outl(dcbase + DC_HDISPLAY, fbmodes[mode].hfl << 16 | fbmodes[mode].hr);	//配置行列像素同步信息
	outl(dcbase + DC_HSYNC_CONF, 0x40000000 | fbmodes[mode].hse << 16 | fbmodes[mode].hss);
	outl(dcbase + DC_VDISPLAY, fbmodes[mode].vfl << 16 | fbmodes[mode].vr);
	outl(dcbase + DC_VSYNC_CONF, 0x40000000 | fbmodes[mode].vse << 16 | fbmodes[mode].vss);
	
	fb_base = PHY_TO_UNCACHED(FB_BASE);
	fb_xsize = FB_XSIZE;
	fb_ysize = FB_YSIZE;
	/* 根据配置设置像素色深以及打开显示输出 */
#if VIDEO_32BPP
	fb_bbp = 32;
	outl(dcbase + DC_FB_STRIDE, (FB_XSIZE*4 + 0xff) & ~0xff);					
	outl(dcbase + DC_FB_CONF, 0x00100104);					//RGBX888
#elif VIDEO_16BPP 
	fb_bbp = 16;
	outl(dcbase + DC_FB_STRIDE, (FB_XSIZE*2 + 0xff) & ~0xff);					
	outl(dcbase + DC_FB_CONF, 0x00100103);					//RGB565
#elif VIDEO_15BPP
	fb_bbp = 15;
	outl(dcbase + DC_FB_STRIDE, (FB_XSIZE*2 + 0xff) & ~0xff);					
	outl(dcbase + DC_FB_CONF, 0x00100102);					//RGB555
#elif VIDEO_12BPP
	fb_bbp = 12;
	outl(dcbase + DC_FB_STRIDE, (FB_XSIZE*2 + 0xff) & ~0xff);					
	outl(dcbase + DC_FB_CONF, 0x00100101);					//RGB444
#else
	outl(dcbase + DC_FB_CONF, 0x00);						//disable
	printf("display controller init failed, unsupported graphic data format!\n");	
	return -1;
#endif
	
	return 0;
}

