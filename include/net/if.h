
/**
 * Copyright(c) 2017-7-26 Shangwen Wu	
 *
 * 网络接口相关定义
 * 
 */

#ifndef __NET_IF_H__
#define __NET_IF_H__

struct mbuf;
struct list_head;
struct rtentry;
struct sockaddr;
struct socket;

struct if_data {
	/* 接口描述信息 */
	uchar ifd_type;									/* 接口类型 */
	uchar ifd_addrlen;								/* 接口地址长度 */
	ulong ifd_mtu;									/* 接口最大传输单元 */
	ulong ifd_metric;								/* 接口转发数据包时需要消耗的ttl跳数 */
	/* 统计信息 */
	ulong ifd_ipackets;								/* 输入数据包个数 */
	ulong ifd_opackets;								/* 输出数据包个数 */
	ulong ifd_ibytes;								/* 输入字节数 */
	ulong ifd_obytes;								/* 输出字节数 */
	ulong ifd_oerrors;								/* 发送错误包数 */
	ulong ifd_ierrors;								/* 接收错误包数 */
};

#define IFNAMESZ	16

#define IFQ_MAXLEN	50

/* if队列 */
struct ifqueue {
	struct mbuf *ifq_head, *ifq_tail;	//tail是否为NULL作为ifqueue是否为空的判断条件，初始化时两者必须为NULL
	ulong ifq_len, ifq_maxlen;
	ulong ifq_drops;
};

#define IFQ_ISFULL(q) 	((q)->ifq_len >= (q)->ifq_maxlen)
#define IFQ_ISEMPTY(q) 	(!(q)->ifq_len && !(q)->ifq_head)

#define IFQ_DROP(q) \
do { \
	++(q)->ifq_drops; \
} while(0)

#define IFQ_ENQUEUE(q, m) \
do { \
	(m)->m_nextpkt = NULL; \
	if(NULL == (q)->ifq_tail) \
		(q)->ifq_head = (m); \
	else \
		(q)->ifq_tail->m_nextpkt = (m); \
	(q)->ifq_tail = (m); \
	++(q)->ifq_len; \
} while(0)

#define IFQ_PREPEND(q, m) \
do { \
	(m)->m_nextpkt = (q)->ifq_head; \
	if(NULL == (q)->ifq_tail) \
		(q)->ifq_tail = (m); \
	(q)->ifq_head = (m); \
	++(q)->ifq_len; \
} while(0) 

#define IFQ_DEQUEUE(q, m) \
do { \
	(m) = (q)->ifq_head; \
	if(m) { \
		if(NULL == ((q)->ifq_head = (m)->m_nextpkt)) \
			(q)->ifq_tail = NULL; \
		(m)->m_nextpkt = NULL; \
		--(q)->ifq_len; \
	} \
} while(0)

/* 接口描述 */
struct ifnet {
	char if_xname[IFNAMESZ];						/* 外部接口名称，name + unit */
	struct list_head if_addrlist;					/* 当前接口所包含的地址链表 */
	struct list_head if_list;						/* 全局的接口链表节点 */
	ushort if_index;								/* 全局接口索引 */
	ushort if_flags;								/* 标志位 */
	void *if_pri;									/* 私有数据 */
	struct if_data if_data;							/* 接口信息以及统计信息 */

	int (*if_output)(struct ifnet *, struct mbuf *, struct sockaddr *, struct rtentry *);
	int (*if_ioctl)(struct ifnet *, unsigned long, caddr_t);
	int (*if_start)(struct ifnet *);

	struct ifqueue if_sndq;							/* 发送数据队列 */
};

#define if_type if_data.ifd_type
#define if_addrlen if_data.ifd_addrlen
#define if_mtu if_data.ifd_mtu
#define if_metric if_data.ifd_metric
#define if_ipackets if_data.ifd_ipackets
#define if_opackets if_data.ifd_opackets
#define if_ibytes if_data.ifd_ibytes
#define if_obytes if_data.ifd_obytes
#define if_oerrors if_data.ifd_oerrors
#define if_ierrors if_data.ifd_ierrors

#define IFF_UP				0x0001		/* 接口UP */
#define IFF_BROADCAST		0x0002		/* 接口支持广播 */
#define IFF_LOOPBACK		0x0004		/* 回环接口 */
#define IFF_POINT2POINT		0x0008		/* 点对点接口 */
#define IFF_RUNNING			0x0010		/* 物理链接已经链上 */
#define IFF_MULTICAST		0x0020		/* 端口支持多播 */
#define IFF_SIMPLEX			0x0040		/* 当该标识置位时，自己将收到自己发送的广播报文 */
#define IFF_NOARP			0x0080		/* 不支持ARP地址解析 */
#define IFF_PROMISC			0x0100		/* 混杂模式 */

#define IFF_CANTCHANGE		(IFF_BROADCAST | IFF_POINT2POINT | IFF_MULTICAST | IFF_RUNNING)

/* 接口地址描述 */
struct ifaddr {
	struct sockaddr *ifa_addr;						/* 当前接口地址 */
	struct sockaddr *ifa_dstaddr;					/* 对于点对点接口时，对端的地址 */
#define ifa_broadaddr ifa_dstaddr					/* 接口广播地址 */
	struct sockaddr *ifa_netmask;					/* 子网掩码 */
	struct ifnet *ifa_ifp;							/* 指向接口地址所属的接口结构 */
	struct list_head ifa_list;						/* 地址链表节点 */
	ulong ifa_refcnt;								/* 引用计数 */
	ulong ifa_metric;								/* 网络TTL */
	void (*ifa_rtrequest)(int, struct rtentry *, struct sockaddr *);//ifaddr级别的路由操作函数，将被request调用
	ushort ifa_flags;								/* 标志位 */
};

#define IFA_ROUTED			RTF_UP					/* 标志该ifaddr是否已经加入到路由 */	

/* ioctl配置请求结构 */
struct ifreq {
	char ifr_name[IFNAMESZ];
	union {
		struct sockaddr ifru_addr;
		struct sockaddr ifru_dstaddr;
		struct sockaddr ifru_broadaddr;
		ushort ifru_flags;
		ulong ifru_metric;
		caddr_t ifru_data;
	} ifr_u;
#define ifr_addr ifr_u.ifru_addr
#define ifr_dstaddr ifr_u.ifru_dstaddr
#define ifr_broadaddr ifr_u.ifru_broadaddr
#define ifr_flags ifr_u.ifru_flags
#define ifr_data ifr_u.ifru_data
#define ifr_metric ifr_u.ifru_metric
};

struct ifaliasreq {
	char ifru_name[IFNAMESZ];
	struct sockaddr ifar_addr;
	struct sockaddr ifar_dstaddr;
#define ifar_broadaddr ifar_dstaddr
	struct sockaddr ifar_mask;
};

/* 用于SIOCGIFCONF ioctl命令，获取当前系统下所有网络接口名称以及其地址信息  */
struct ifconf {
	ulong ifc_len;
	union {
		caddr_t ifcu_buf;
		struct ifreq *ifcu_req;	
	} ifc_u;
#define ifc_req ifc_u.ifcu_req
#define ifc_buf ifc_u.ifcu_buf
};

typedef int (*ifcallback_t)(struct ifnet *, void *);

extern int if_attach(struct ifnet *, int);			//defined in if.c
extern struct ifaddr *ifa_ifwithdstaddr(struct sockaddr *addr);
extern struct ifaddr *ifa_ifwithnet(struct sockaddr *addr);
extern struct ifaddr *ifa_ifwithaddr(struct sockaddr *addr);
extern int if_callback_foreach(ifcallback_t func, void *arg);
extern int if_callback_foreach_up(ifcallback_t func, void *arg);
extern int if_callback_foreach_name(ifcallback_t func, const char *ifname, void *arg);
extern struct ifnet *if_find_byname(const char *ifname);
extern int if_ioctl(struct socket *so, unsigned long cmd, caddr_t arg);
extern int if_getconf(struct ifconf *);		
extern struct ifaddr *ifaof_ifpforaddr(struct sockaddr *dst, struct ifnet *ifp);

#endif //__NET_IF_H__
