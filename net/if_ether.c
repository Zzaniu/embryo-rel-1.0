
/**
 * Copyright(c) 2018-8-16 Shangwen Wu	
 *
 * 以太网层相关实现
 * 
 */
#include <common.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/list.h>
#include <sys/err.h>
#include <sys/endian.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/netisr.h>
#include <mach/intr.h>

#include <net/if_dl.h>
#include <net/if.h>
#include <net/if_type.h>
#include <net/route.h>
#include <net/if_ether.h>

#include <netinet/in.h>
#include <netinet/in_arp.h>

uint8_t ether_broadaddr[ETH_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static int ether_output(struct ifnet *, struct mbuf *, struct sockaddr *, struct rtentry *);

/**
 * 描述：以太网层输入函数，该函数被驱动程序调用，该函数将解析硬件头，并根据报文类型
 * 		 将数据报文去掉硬件头后，分别插入到对应的协议层队列中	
 */
int ether_input(struct ifnet *ifp, struct mbuf *m)
{
	int s;
	struct ether_hdr *ethhdr;
	struct ifqueue *ifq;
	struct ether_dev *ethdev = (struct ether_dev *)ifp;
	extern struct ifqueue ipintrq;	//defined in ip_input.c
	extern struct ifqueue arpintrq;	//defined in arp.c

	ifp->if_ibytes += m->m_pkthdr.mp_len;
	/* 包个数需要驱动确认是否发送出错，该字段由驱动设置 */
	//++ifp->if_ipackets;

	/* 解析并去掉硬件头 */
	ethhdr = mtod(m, struct ether_hdr *);
	m->m_data += sizeof(struct ether_hdr);
	m->m_pkthdr.mp_len -= sizeof(struct ether_hdr);
	m->m_len -= sizeof(struct ether_hdr);
	
	/* 标记多播和广播报文 */
	if(ethhdr->ehd_dst[0] & 0x1) {
		if(!bcmp(ethhdr->ehd_dst, ether_broadaddr, ETH_ADDR_LEN))
			m->m_flags |= MF_BCAST;
		else
			m->m_flags |= MF_MCAST;
	}

	/* 非混杂模式下，丢弃不是本机的单播报文 */
	if(!(m->m_flags & (MF_BCAST | MF_MCAST)) && !(ifp->if_flags & IFF_PROMISC)) {
		if(bcmp(ethdev->eth_addr, ethhdr->ehd_dst, ETH_ADDR_LEN))
			goto drop;	
	}

	/* 判断上层协议类型，并将数据报文插入到对应协议的接收队列中 */
	switch(htons(ethhdr->ehd_pro)) {
		case ETHERTYPE_IP:
			ifq = &ipintrq;		
			schednetisr(NETISR_IP); //bad code，设置中断标志位应当在插入队列之后进行
			break;
		case ETHERTYPE_ARP:
			if(ifp->if_flags & IFF_NOARP)
				goto drop;
			ifq = &arpintrq;		
			schednetisr(NETISR_ARP);
			break;
		default:
			log(LOG_ERR, "ether_input: %s don't support ethertype %d\n", ifp->if_xname, htons(ethhdr->ehd_pro));
			goto drop;
	}

	s = splimp();
	/* 插入IP接收队列 */
	if(IFQ_ISFULL(ifq)) {
		log(LOG_ERR, "IP/ARP recv queue is full\n");
		IFQ_DROP(ifq);
		splx(s);
		goto drop;
	}
	
	IFQ_ENQUEUE(ifq, m);

	splx(s);

	return 0;

drop:
	mbuf_freem(m);
	return -1;
}

/**
 * 描述：分配一个ether_dev结构，并进行以太网相关设置
 *
 */
struct ether_dev *alloc_etherdev(char *ifname, uint8_t *dev_addr)
{
	int err;
	struct ether_dev *ethdev;
	struct ifnet *ifp;
	struct ifaddr *ifa;
	struct sockaddr_dl *sdl;

	if(NULL == (ethdev = (struct ether_dev *)kmem_zmalloc(sizeof(struct ether_dev)))) {
		log(LOG_ERR, "alloc_etherdev: out of memory\n");
		return ERR_PTR(ENOMEM);
	}
	ifp = &ethdev->eth_if;	
	bcopy(ifname, ifp->if_xname, IFNAMESZ);

	ether_setup(ethdev);
	if((err = if_attach(ifp, 1)) != 0) {
		kmem_free(ethdev);
		return ERR_PTR(err);
	}
	bcopy(dev_addr, ethdev->eth_addr, ETH_ADDR_LEN);
	
	/* 修正ifnet中部分链路层的地址信息，并将MAC地址拷贝到ifnet中链路层地址结构中 */
	list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) { 
		if((sdl = (struct sockaddr_dl *)ifa->ifa_addr) && AF_LINK == sdl->sdl_family) {
			sdl->sdl_type = ifp->if_type;
			sdl->sdl_alen = ifp->if_addrlen;
			bcopy(ethdev->eth_addr, LLADDR(sdl), sdl->sdl_alen);
			break;
		}
	}

	return ethdev;
}

/**
 * 描述：释放一个ether_dev结构的资源
 */
void free_etherdev(struct ether_dev *ethdev)
{
	//bad code这里还需要添加ifnet相关的删除操作

	kmem_free(ethdev);
}

/**
 * 描述：设置以太网相关字段
 */
void ether_setup(struct ether_dev *ethdev)
{
	struct ifnet *ifp = &ethdev->eth_if;

	ifp->if_type = IFT_ETHER;
	ifp->if_addrlen = ETH_ADDR_LEN;			//注意这个字段要在if_attach之前初始化
	ifp->if_mtu = ETHERMTU;
	ifp->if_output = ether_output;
}

/**
 * 描述：flush发送队列
 */
static void ether_flush_if_sndq(struct ifnet *ifp)
{
	while(!IFQ_ISEMPTY(&ifp->if_sndq)) 
		(*ifp->if_start)(ifp);
}

/**
 * 描述：以太网层输出函数，该函数将插入以太网首部，并调用网卡驱动接口进行数据报文的发送，该函数将检查
 * 		 目的MAC是否在ARP缓存表中，如果没有找到匹配的ARP表项，将发送ARP请求报文进行地址解析
 *
 */
static int ether_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst, struct rtentry *rt0)
{
	int err = 0, s;
	uint16_t ethertype;
	struct mbuf *lo_m = NULL;
	uint8_t macaddr[ETH_ADDR_LEN] = {0};
	struct ether_hdr *ethhdr;
	struct rtentry *rt = NULL;
	struct ether_dev *ethdev = (struct ether_dev *)ifp;
	extern struct ifnet *lo0;

#define senderr(x) do {err = (x); goto failed;} while(0)

	ETHLVL_DBG("ether output...\n");

	//if((ifp->if_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
	if((ifp->if_flags & (IFF_UP)) != (IFF_UP))
		return ENETDOWN;

	/* 检查是否传入路由，如果通过ip_output等协议栈函数调用该函数，那么其rt0将为包含链路信息的专属路由 */
	if((rt = rt0) != NULL) {
		if(!(rt->rt_flags & RTF_UP)) {
			/* 路由无效需要进行重新匹配路由 */
			if((rt = rt0 = __rt_alloc(dst, 1)) != NULL)
				--rt->rt_refcnt;		//注意这里不对rt进行占用
			else 
				senderr(EHOSTUNREACH);
		}
		if(rt->rt_flags & RTF_GATEWAY) {
			if(!rt->rt_gwroute)
				goto lookup;
			if(!((rt = rt->rt_gwroute)->rt_flags & RTF_UP)) {
				;//rtfree(rt);bad code 
				rt = rt0;
lookup:
				rt->rt_gwroute = __rt_alloc(dst, 1);	//rt_refcnt--???
				if(!(rt = rt->rt_gwroute))
					senderr(EHOSTUNREACH);
			}
		}
		if((rt->rt_flags & RTF_REJECT) && \
			(!rt->rt_expire))
			senderr(rt == rt0 ? EHOSTDOWN : EHOSTUNREACH);
	}

	switch(dst->sa_family) {
		case AF_INET:
			if(arpresolve(ethdev, dst, m, rt, macaddr) != 0) 
				return 0;		//这里直接返回0，m将在arpresolve中被释放或者保存在arp表中
			/* 如果目的端是广播地址，并且本地接口设置了SIMPLEX标识，那么将本数据包往回环接口也发送一份 */
			if((m->m_flags & MF_BCAST) && (ifp->if_flags & IFF_SIMPLEX))
				lo_m = mbuf_copy(m, 0, M_COPYALL);
			ethertype = htons(ETHERTYPE_IP);
			break;
		case AF_UNSPEC:
			ethhdr = (struct ether_hdr *)dst->sa_data;
			bcopy(ethhdr->ehd_dst, macaddr, ETH_ADDR_LEN);
			ethertype = ethhdr->ehd_pro;			
			break;
		default:
			log(LOG_ERR, "%s can't handle af %d\n", ifp->if_xname, dst->sa_family);
			senderr(EAFNOSUPPORT);
	}
	/* 往自己也送一份数据报文 */
	if(lo_m != NULL)
		(*lo0->if_output)(lo0, lo_m, dst, rt);
	
	/* 插入以太网帧头 */
	if(NULL == (m = mbuf_reserve(m, sizeof(struct ether_hdr))))
		return ENOBUFS;	//mbuf_reserve将会释放m空间	
	ethhdr = mtod(m, struct ether_hdr *);
	bcopy(macaddr, ethhdr->ehd_dst, ETH_ADDR_LEN);
	bcopy(ethdev->eth_addr, ethhdr->ehd_src, ETH_ADDR_LEN);
	ethhdr->ehd_pro = ethertype;
	
	/* 将数据报文插入到发送队列 */
	s = splimp();
	/* 插入IP接收队列 */
	if(IFQ_ISFULL(&ifp->if_sndq)) {
		log(LOG_ERR, "%s: send queue is full\n", ifp->if_xname);
		IFQ_DROP(&ifp->if_sndq);
		/* 注意：当发送队列满时，强制flash队列，防止队列里的报文永远发不出去 */
		ether_flush_if_sndq(ifp);
		splx(s);
		senderr(ENOBUFS);
	}
	IFQ_ENQUEUE(&ifp->if_sndq, m);

	ifp->if_obytes += m->m_pkthdr.mp_len;
	/* 包个数需要驱动确认是否发送出错，该字段由驱动设置 */
	//++ifp->if_opackets;

	/* 调用驱动层发送函数 */
	(*ifp->if_start)(ifp);

	splx(s);

	return 0;

failed:
	if(m)
		mbuf_freem(m);

	return err;
}

