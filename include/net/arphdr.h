
/**
 * Copyright(c) 2018-8-20 Shangwen Wu	
 *
 * ARP协议头相关定义（通用定义）
 * 
 */

#ifndef __NET_ARPHDR_H__
#define __NET_ARPHDR_H__

/* ARP地址解析协议头 */
struct arphdr {
	uint16_t ar_hrd;			/* 硬件地址类型 */
#define ARPHRD_ETHER	1							/* 以太网硬件格式 */
#define ARPHRD_IEEE802	6							/* IEEE802硬件格式 */
	uint16_t ar_pro;			/* 协议地址类型 */
	uint8_t ar_hlen;			/* 硬件地址长度 */
	uint8_t ar_plen;			/* 协议地址长度 */
	uint16_t ar_op;				/* ARP操作类型 */
#define ARPOP_REQUEST		1	/* 根据协议地址解析硬件地址 */
#define ARPOP_REPLY			2	/* ARPOP_REQUEST响应报文 */
#define ARPOP_REVREQUEST	3	/* 根据硬件地址解析协议地址 */
#define ARPOP_REVREPLY		4	/* ARPOP_REVREQUEST响应报文 */
};

#endif //__NET_ARPHDR_H__
