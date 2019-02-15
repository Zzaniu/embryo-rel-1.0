
/**
 * Copyright(c) 2018-8-16 Shangwen Wu	
 *
 * 以太网层相关定义
 * 
 */

#ifndef __NET_IF_ETHER_H__
#define __NET_IF_ETHER_H__

struct ifnet;
struct mbuf;

#define ETHERMTU		1500			//以太网MTU
#define ETH_ADDR_LEN	6				//MAC地址长度
#define ETH_DATA_LEN	ETHERMTU
#define ETH_FCS_LEN		4
#define ETH_HEADER_LEN	14
#define ETH_FRAME_LEN	(ETH_DATA_LEN + ETH_FCS_LEN + ETH_HEADER_LEN)

/* 根据IP地址得到对应的多播以太网地址 */
#define ETHER_MAP_IP_MUILTCAST(ipaddr, macaddr)	\
do {											\
	(macaddr)[0] = 0x01;						\
	(macaddr)[1] = 0x00;						\
	(macaddr)[2] = 0x5e;						\
	(macaddr)[3] = ((uint8_t *)ipaddr)[1] & 0x7f;	\
	(macaddr)[4] = ((uint8_t *)ipaddr)[2];		\
	(macaddr)[5] = ((uint8_t *)ipaddr)[3];		\
} while(0)

struct ether_dev {
	struct ifnet eth_if;				//ifnet结构，该结构必须在ether_dev第一个字段位置
	uint8_t eth_addr[ETH_ADDR_LEN];		//MAC地址
	void *eth_intrhandler;				//中断处理函数
};

/* 以太网头定义 */
struct ether_hdr {
	uint8_t ehd_dst[ETH_ADDR_LEN];
	uint8_t ehd_src[ETH_ADDR_LEN];
	uint16_t ehd_pro;					/* 上层协议 */
#define ETHERTYPE_IP	0x0800			/* IP网络协议 */
#define ETHERTYPE_ARP	0x0806			/* ARP地址解析协议 */
#define ETHERTYPE_RARP	0x8035			/* RARP逆地址协议 */
};

/* ETH层调试开关 */
#define ETHLVL_DEBUG		0
#if ETHLVL_DEBUG
#define ETHLVL_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define ETHLVL_DBG(fmt, args...)		do{}while(0)	
#endif

static inline is_multicast_ether_addr(uint8_t *addr)
{
	return (addr[0] & 0x01);
}

static inline is_zero_ether_addr(uint8_t *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

static inline is_valid_ether_addr(uint8_t *addr)
{
	return (!is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr)); 
}

extern struct ether_dev *alloc_etherdev(char *ifname, uint8_t *dev_addr);//defined in if_ether.c
extern void free_etherdev(struct ether_dev *ethdev);
extern void ether_setup(struct ether_dev *ethdev);
extern int ether_input(struct ifnet *ifp, struct mbuf *m);

extern uint8_t ether_broadaddr[ETH_ADDR_LEN];

#endif //__NET_IF_ETHER_H__
