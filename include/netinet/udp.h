
/**
 * Copyright(c) 2018-9-10 Shangwen Wu	
 *
 * UDP首部相关定义
 * 
 */

#ifndef __NETINET_UDP_H__
#define __NETINET_UDP_H__

/* UDP报文首部格式 */
struct udp {
	uint16_t udp_sport;	
	uint16_t udp_dport;	
	uint16_t udp_ulen;	
	uint16_t udp_cksum;
};

/* IP+UDP报文头 */
struct udpip {
	struct ipovly ui_ipv;
	struct udp ui_udp;
};

#define ui_sport ui_udp.udp_sport
#define ui_dport ui_udp.udp_dport
#define ui_ulen ui_udp.udp_ulen
#define ui_cksum ui_udp.udp_cksum
#define ui_resv ui_ipv.ipv_resv
#define ui_pro ui_ipv.ipv_pro
#define ui_len ui_ipv.ipv_len
#define ui_src ui_ipv.ipv_src
#define ui_dst ui_ipv.ipv_dst

#endif //__NETINET_UDP_H__
