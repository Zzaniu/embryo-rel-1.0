
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * internet 控制块相关定义
 * 
 */

#ifndef __NETINET_IN_PCB_H__
#define __NETINET_IN_PCB_H__

#define socktoinpcb(so)  (struct in_pcb *)((so)->soc_pcb)

struct socket;
struct ip;
struct in_pcb;
struct in6_addr;	//defined in in.h
struct in_addr;		//defined in in.h
struct route;

struct inpcbtable {
	struct list_head int_pcbqueue;			//inpcb队列
	struct list_head *int_pcbhashtbl;		//便于快速查询的inpcb哈希表
	ulong int_hashmask;						//用于确定哈希表索引的掩码
	ulong int_lastport;						//最后一个端口
};

#define INP_HDRINCL			0x0001	/* 协议头已经由用户传递的mbuf中定义好 */
#define INP_HIPORT			0x0002	/* 大端口区间 */
#define INP_LOPORT			0x0004	/* 低端口区间 */
#define INP_CONTROLOPS		0x0008	/* 应用层要求返回控制数据 */

/* inpcb统一地址描述 */
union inpaddru {
	struct in6_addr iau_addr6;
	struct {
		uint8_t pad[12];
		struct in_addr inaddr;
	} iau_a4u;
};

/**
 * 用于建立数据报文与socket之间的绑定关系
 */
struct in_pcb {
	ushort inp_flags;
	struct socket *inp_sock;
	struct inpcbtable *inp_tlb;
	struct list_head inp_queue;
	struct list_head inp_hash;
	struct route inp_ro;
	union {
		struct ip hu_ip;
	} inp_hu;					/* 协议数据头 */
#define inp_ip	inp_hu.hu_ip
	struct mbuf *inp_option;
	union inpaddru inp_faddru;	/* 绑定的对端地址 */
	union inpaddru inp_laddru;	/* 绑定的本地地址 */
#define inp_faddr	inp_faddru.iau_a4u.inaddr
#define inp_laddr	inp_laddru.iau_a4u.inaddr
#define inp_faddr6	inp_faddru.iau_addr6
#define inp_laddr6	inp_laddru.iau_addr6
	uint16_t inp_fport;			/* 绑定的对端端口 */
	uint16_t inp_lport;			/* 绑定的本地端口 */
};

#define INPCBLOOKUP_WILDCARD		1			/* 提示lookup函数是否进行同陪查找 */

extern void init_inpcb(struct inpcbtable *table, ulong hashsize);		//defined in in_pcb.c
extern int alloc_inpcb(struct socket *so, struct inpcbtable *tlbp); 	//defined in in_pcb.c
extern void free_inpcb(struct socket *so);
extern int inpcb_bind(struct in_pcb *inp, struct mbuf *addr);
extern int inpcb_connect(struct in_pcb *inp, struct mbuf *addr);
extern void inpcb_disconnect(struct in_pcb *inp);
extern void inpcb_rehash(struct in_pcb *inp);
extern struct in_pcb *inpcb_hashlookup(struct inpcbtable *table, 
			struct in_addr faddr, uint16_t fport,
			struct in_addr laddr, uint16_t lport);
extern struct in_pcb *inpcb_lookup(struct inpcbtable *table, int flags,
			struct in_addr faddr, uint16_t fport,
			struct in_addr laddr, uint16_t lport);

#endif //__NETINET_IN_PCB_H__
