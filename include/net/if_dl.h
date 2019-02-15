
/**
 * Copyright(c) 2017-8-2 Shangwen Wu	
 *
 * 数据链路曾相关定义
 * 
 */

#ifndef __NET_IF_DL_H__
#define __NET_IF_DL_H__

/* 数据链路曾地址结构 */
struct sockaddr_dl {
	unsigned char 	sdl_len;					//结构总长度
	sa_family_t 	sdl_family;					//网络域
	uint16_t 		sdl_index;					//链路曾接口索引
	uint8_t			sdl_type;					/* 接口类型 */
	uint8_t 		sdl_nlen;					//接口名称长度
	uint8_t			sdl_alen;					//接口地址长度
	char 			sdl_data[12];				//其他数据，该部分包括接口名称以及addr，该长度最小为12
};

#define satosdl(sa)	((struct sockaddr_dl *)(sa))
#define LLADDR(sdl)	((caddr_t)((sdl)->sdl_data + (sdl)->sdl_nlen))

#endif //__NET_IF_DL_H__
