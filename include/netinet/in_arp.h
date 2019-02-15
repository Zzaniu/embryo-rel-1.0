
/**
 * Copyright(c) 2018-8-19 Shangwen Wu	
 *
 * ARP协议层相关定义
 * 
 */

#ifndef __NETINET_IN_ARP_H__
#define __NETINET_IN_ARP_H__

#include <net/arphdr.h>

struct ether_dev;
struct ifaddr;
struct mbuf;
struct rtentry;
struct list_head;

/* ARP-IP完整报文格式 */
struct ip_arp {
	struct arphdr ipa_hdr;							/* 通用协议头 */
	uint8_t ipa_sha[ETH_ADDR_LEN];					/* 发送端以太网地址 */
	uint8_t ipa_spa[sizeof(struct in_addr)];		/* 发送端IP地址 */
	uint8_t ipa_dha[ETH_ADDR_LEN];					/* 目的以太网地址 */
	uint8_t ipa_dpa[sizeof(struct in_addr)];		/* 目的IP地址 */
#define ipa_hrd		ipa_hdr.ar_hrd
#define ipa_pro		ipa_hdr.ar_pro
#define ipa_hlen	ipa_hdr.ar_hlen
#define ipa_plen	ipa_hdr.ar_plen
#define ipa_op		ipa_hdr.ar_op
};

struct llinfo_arp {
	struct list_head la_list;	//la链表
	struct rtentry *la_rt;
	struct mbuf *la_hold;		//引起地址解析的数据报文
	ulong la_asked;				//表示已经发起ARP请求的次数，用于防止ARP风暴
#define la_timer la_rt->rt_expire	//记录ARP地址信息有效时间
};

/* ARP层调试开关 */
#define ARPLVL_DEBUG		0
#if ARPLVL_DEBUG
#define ARPLVL_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define ARPLVL_DBG(fmt, args...)		do{}while(0)	
#endif

/* defined in arp.c */
extern void arprequest(struct ether_dev *ethdev, in_addr_t *sip, in_addr_t *tip, uint8_t *macaddr);
extern int arpresolve(struct ether_dev *ethdev, struct sockaddr *dst, 
		struct mbuf *m, struct rtentry *rt, uint8_t *macaddr);
extern struct llinfo_arp *arplookup(in_addr_t addr, int report);
extern void arp_ifinit(struct ether_dev *ethdev, struct ifaddr *ifa);
extern int in_arpinput(struct mbuf *m);

#endif //__NETINET_IN_ARP_H__
