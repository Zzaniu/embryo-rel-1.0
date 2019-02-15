
/**
 * Copyright(c) 2018-8-19 Shangwen Wu	
 *
 * ARP相关实现
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
#include <sys/syslog.h>
#include <sys/errno.h>
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
#include <netinet/in_var.h>
#include <netinet/in_arp.h>

#include <arpa/inet.h>

/* llinfo_arp全局链表 */
static LIST_HEAD(llinfo_arp_list); 

/* ARP协议层接收队列 */
struct ifqueue arpintrq = {
	.ifq_head = NULL, 
	.ifq_tail = NULL,
	.ifq_len = 0,
	.ifq_maxlen = IFQ_MAXLEN,
	.ifq_drops = 0
};

static char *ether_mactostr(const uint8_t *mac);

/**
 * 描述：ARP报文软件中断处理程序
 */
void arpintr(void)
{
	int s;
	struct mbuf *m;
	struct ip_arp *ipa;

	ARPLVL_DBG("In arpintr\n");

	while(1) {
		s = splimp();
		IFQ_DEQUEUE(&arpintrq, m);
		splx(s);
		if(!m)
			return;
		if(!(m->m_flags & MF_PKTHDR))
			panic("arpintr: mbuf has no pakhdr");
		/* 检查ARP报文是否为IP-ETHER格式报文 */	
		if((m->m_len >= sizeof(struct ip_arp))
			&& (ipa = mtod(m, struct ip_arp *)) 
			&& (ARPHRD_ETHER == htons(ipa->ipa_hrd))) {
			switch(htons(ipa->ipa_pro)) {
				case ETHERTYPE_IP:
					in_arpinput(m);
					continue;
			}
		}
		mbuf_freem(m);
	}
}

/**
 * 描述：ARP协议层输入函数，该函数用于处理ARP请求和响应，并更新对应的ARP表信息
 * 注意：该函数仅处理以太网地址与IP地址之间的地址解析
 */
int in_arpinput(struct mbuf *m)
{
	int match = 0;
	struct ip_arp *ipa;
	struct in_ifaddr *ia, *maybe_ia = NULL;
	struct in_addr siaddr, tiaddr, myiaddr;
	struct ifnet *ifp = m->m_pkthdr.mp_recvif;
	struct ether_dev *ethdev = (struct ether_dev *)ifp;
	struct llinfo_arp *la;
	struct sockaddr_dl *sdl;
	struct rtentry *rt;
	struct ether_hdr *ethhdr;
	struct sockaddr sa;
	
	ipa = mtod(m, struct ip_arp *);
	bcopy(ipa->ipa_dpa, (caddr_t)&tiaddr.s_addr, sizeof(ipa->ipa_dpa));
	bcopy(ipa->ipa_spa, (caddr_t)&siaddr.s_addr, sizeof(ipa->ipa_spa));

	/* 查找接收到ARP报文的本地地址接口 */
	list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
		if(ia->ia_ifp == ifp) {
			maybe_ia = ia;
			if((ia->ia_addr.sin_addr.s_addr == tiaddr.s_addr) 
				|| (ia->ia_addr.sin_addr.s_addr == siaddr.s_addr)) {
				match = 1;
				break;
			}
		}
	}
	/* 接收接口未配置任何IP */
	if(NULL == maybe_ia) 
		goto drop;
	
	/* 获取接收到ARP报文的本地接口IP地址，优先选择匹配于ARP目的/源地址的本地IP */
	myiaddr = (1 == match) ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;
	
	/* 源硬件地址为本机的数据报文直接丢弃，不接收自己发出的ARP报文 */
	if(!bcmp(ipa->ipa_sha, ethdev->eth_addr, sizeof(ipa->ipa_sha)))
		goto drop;

	/* 源硬件地址为广播地址的报文直接丢弃 */
	if(!bcmp(ipa->ipa_sha, ether_broadaddr, sizeof(ipa->ipa_sha))) {
		log(LOG_INFO, "ethernet address is broadcase for IP address %s", 
			inet_ntoa(siaddr));
		goto drop;
	}
	
	/* 检测是否发生IP地址冲突 */
	if(siaddr.s_addr == myiaddr.s_addr) {
		log(LOG_WARNING, "duplicate IP address %s sent from ethernet address %s\n", 
			inet_ntoa(siaddr), ether_mactostr(ipa->ipa_sha));
		tiaddr = myiaddr;
		goto reply;
	}
	
	/** 
 	 * 获取源IP地址的专属路由，注意如果对方的目的IP与接收到的本地接口IP不匹配（即不是直接发送给本机的报文），
 	 * 那么不会为该报文的源IP创建新的专属路由（ARP表项），但是会查询已有的与源IP地址匹配的专属路由（ARP表项）
 	 */
	la = arplookup(siaddr.s_addr, tiaddr.s_addr == myiaddr.s_addr);
	/* 如果能找到ARP表项，则需要对路由表项进行更新或者添加硬件地址 */
	if(la && (rt = la->la_rt) && (sdl = satosdl(rt->rt_gateway))) {
		/* 新收到的ARP信息与原ARP表中数据不同，需要更新之前的硬件地址信息 */
		if(sdl->sdl_alen && 
			bcmp(LLADDR(sdl), ipa->ipa_sha, sizeof(ipa->ipa_sha))) {
			/* 固定ARP类型的专属路由以及接收接口与发送接口不同的情况下不会进行地址更新 */
			if(rt->rt_flags & RTF_PERMANENT_ARP) {
				log(LOG_WARNING, "arp: attempt to overwrite permanent"
					" entry for %s by %s on %s\n",
					inet_ntoa(siaddr), ether_mactostr(ipa->ipa_sha), 
					ifp->if_xname);
				goto drop;
			} else if(rt->rt_ifp != ifp) {
				log(LOG_WARNING, "arp: attempt to overwrite"
					" entry for %s on %s by %s on %s\n",
					inet_ntoa(siaddr), rt->rt_ifp->if_xname,
					ether_mactostr(ipa->ipa_sha), ifp->if_xname);
				goto drop;
			} else {
				log(LOG_INFO, "arp: info overwritten"
					" for %s by %s on %s\n",
					inet_ntoa(siaddr), ether_mactostr(ipa->ipa_sha), 
					ifp->if_xname);
			}
			
		}
		/* 更新解析的硬件地址 */
		bcopy(ipa->ipa_sha, LLADDR(sdl), sdl->sdl_alen = sizeof(ipa->ipa_sha));
		/* 刷新ARP表的有效时间 */
		la->la_asked = 0;
		rt->rt_flags &= ~RTF_REJECT;
		if(rt->rt_expire)
			rt->rt_expire = 0;		//bad code 临时代码，这里需要将rt_expire设置为当前时间+有效时间
		/* 如果有因为之前地址解析而被挂起的数据报文，这里得到对应的硬件地址后，将自动发送该挂起的报文 */
		if(la->la_hold) {
			(*ifp->if_output)(ifp, la->la_hold, rt_key(rt), rt);
			la->la_hold = NULL;	
		}
	}

reply:
	/* 处理ARP请求报文，发送响应报文 */
	if(htons(ipa->ipa_op) != ARPOP_REQUEST) {
		mbuf_freem(m);
		return 0;
	}

	/* 组装响应报文 */
	bcopy(ipa->ipa_sha, ipa->ipa_dha, sizeof(ipa->ipa_dha));
	if(tiaddr.s_addr == myiaddr.s_addr) {
		/* 如果请求解析的MAC地址位于当前接收接口 */
		bcopy(ethdev->eth_addr, ipa->ipa_sha, sizeof(ipa->ipa_sha));
	} else {
		/* 如果请求解析的MAC地址不属于当前接收接口，则需要进行ARP代理 */
		la = arplookup(tiaddr.s_addr, 0);
		if(la && (rt = la->la_rt) && (sdl = satosdl(rt->rt_gateway)) 
			&& sdl->sdl_alen) 
			bcopy(LLADDR(sdl), ipa->ipa_sha, sizeof(ipa->ipa_sha));
		else 
			goto drop;	
	}
	bcopy((caddr_t)&tiaddr.s_addr, ipa->ipa_spa, sizeof(ipa->ipa_spa));
	bcopy((caddr_t)&siaddr.s_addr, ipa->ipa_dpa, sizeof(ipa->ipa_dpa));
	ipa->ipa_op = htons(ARPOP_REPLY);

	/* 组装以太网报文 */
	ethhdr = (struct ether_hdr *)sa.sa_data;	
	bcopy(ipa->ipa_dha, ethhdr->ehd_dst, ETH_ADDR_LEN);
	bcopy(ethdev->eth_addr, ethhdr->ehd_src, ETH_ADDR_LEN);
	ethhdr->ehd_pro = htons(ETHERTYPE_ARP);

	sa.sa_family = AF_UNSPEC;	/* 此类型可以避免地址解析步骤，而直接发送该报文 */
	sa.sa_len = sizeof(struct sockaddr);

	(*ifp->if_output)(ifp, m, &sa, NULL);

	return 0;

drop:
	mbuf_freem(m);
	
	return -1;
}

/**
 * 描述：ARP曾对路由相关额外处理
 */
static void arp_rtrequest(int cmd, struct rtentry *rt, struct sockaddr *sa)
{
	struct sockaddr *gate = rt->rt_gateway;
	struct sockaddr_dl sdl_null = {sizeof(struct sockaddr_dl), AF_LINK};
	struct llinfo_arp *la = (struct llinfo_arp *)rt->rt_llinfo;

	/* bad code 这里需要添加定时删除无效ARP路由的处理 */
	
	/* 不对间接路由做任何处理 */
	if(rt->rt_flags & RTF_GATEWAY)
		return;

	switch(cmd) {
		case RTM_ADD:
			/* 仅需要对非主机（到达网络）的路由进行地址解析处理 */
			if(!(rt->rt_flags & RTF_HOST) && satosin(rt_mask(rt))->sin_addr.s_addr != 0xffffffff)
				rt->rt_flags |= RTF_CLONING;
			/* 对于需要进行地址解析的路由，其route->gateway字段将保存链路层信息 */
			if(rt->rt_flags & RTF_CLONING) {
				/* 为保存链路曾地址信息分配足够的空间 */	
				rt_setgate(rt, rt_key(rt), (struct sockaddr *)&sdl_null);
				satosdl(gate)->sdl_type = rt->rt_ifp->if_type;	
				satosdl(gate)->sdl_index = rt->rt_ifp->if_index;
				rt->rt_expire = 1;			//no static	
				break;
			} 
			/* no break，对于主机路由，需要在进行初始化地址时，就需要处理地址信息，因为其没有resolve这一过程 */
		case RTM_RESOLVE:
			if(rt->rt_gateway->sa_family != AF_LINK || rt->rt_gateway->sa_len < sizeof(struct sockaddr_dl))
				log(LOG_WARNING, "arp_rtrequest: bad rt->gateway info\n");
			//rt->rt_expire = 1???
			satosdl(gate)->sdl_type = rt->rt_ifp->if_type;	
			satosdl(gate)->sdl_index = rt->rt_ifp->if_index;
			if(la != NULL)
				break;
			if(NULL == (la = (struct llinfo_arp *)kmem_zmalloc(sizeof(struct llinfo_arp)))) {
				log(LOG_ERR, "arp_rtrequest: alloc llinfo_arp failed, no buffer\n");
				break;
			}
			la->la_rt = rt;
			list_add(&la->la_list, &llinfo_arp_list);
			rt->rt_llinfo = la;
			rt->rt_flags |= RTF_LLINFO;
			/* 如果目的地址为本机地址，则强制将接口设置为回环接口 */
			if(satosin(rt->rt_ifa->ifa_addr)->sin_addr.s_addr == satosin(rt_key(rt))->sin_addr.s_addr) {
				extern struct ifnet *lo0;		//defined in if_loop.c
				rt->rt_expire = 0;		//设置成永久路由
				/* 将本地mac地址写入新建的路由表 */
				bcopy(((struct ether_dev *)rt->rt_ifp)->eth_addr
					, LLADDR(satosdl(gate)), satosdl(gate)->sdl_alen = ETH_ADDR_LEN);
				rt->rt_ifp = lo0;
			}
				
			break;
	}
}

/**
 * 描述：用于发送一条ARP请求报文
 * 参数：sip\tip ARP报文的目的\源IP地址，macaddr为本机MAC地址
 */
void arprequest(struct ether_dev *ethdev, in_addr_t *sip, in_addr_t *tip, uint8_t *macaddr)
{
	struct mbuf *m;
	struct ip_arp *iarp;
	struct ether_hdr *ethhdr;
	struct sockaddr sa;
	struct ifnet *ifp = &ethdev->eth_if;

	/* 分配一个ARP报文缓冲区 */
	if(NULL == (m = mbuf_getpkt(MT_DATA))) {
		log(LOG_ERR, "arprequest: no buffer\n");
		return;
	}

	m->m_len = m->m_pkthdr.mp_len = sizeof(struct ip_arp);
	MH_ALIGN(m, sizeof(struct ip_arp));
	
	/* 组装ARP报文 */
	iarp = mtod(m, struct ip_arp *);
	bzero((caddr_t)iarp, sizeof(struct ip_arp));
	iarp->ipa_hrd = htons(ARPHRD_ETHER);
	iarp->ipa_pro = htons(ETHERTYPE_IP);
	iarp->ipa_hlen = sizeof(iarp->ipa_sha);	
	iarp->ipa_plen = sizeof(iarp->ipa_spa);	
	iarp->ipa_op = htons(ARPOP_REQUEST);
	bcopy(macaddr, iarp->ipa_sha, ETH_ADDR_LEN);
	bcopy((caddr_t)tip, iarp->ipa_dpa, sizeof(in_addr_t));
	bcopy((caddr_t)sip, iarp->ipa_spa, sizeof(in_addr_t));

	/* 组装以太网报文 */
	ethhdr = (struct ether_hdr *)sa.sa_data;	
	bcopy(ether_broadaddr, ethhdr->ehd_dst, ETH_ADDR_LEN);
	bcopy(macaddr, ethhdr->ehd_src, ETH_ADDR_LEN);
	ethhdr->ehd_pro = htons(ETHERTYPE_ARP);

	sa.sa_family = AF_UNSPEC;	/* 此类型可以避免地址解析步骤，而直接发送该报文 */
	sa.sa_len = sizeof(struct sockaddr);

	(*ifp->if_output)(ifp, m, &sa, NULL);
}

/**
 * 描述：ARP解析函数
 * 参数：macaddr必须由上层函数分配空间，并且仅当解析成功时，地址数据有效
 * 返回：返回0表示解析成功，返回-1表示解析失败或者正在发送ARP请求
 * 注意：如果该函数执行失败，将释放m所占空间
 */
int arpresolve(struct ether_dev *ethdev, struct sockaddr *dst, 
		struct mbuf *m, struct rtentry *rt, uint8_t *macaddr)
{
	struct sockaddr_dl *sdl;
	struct llinfo_arp *la;
	struct ifnet *ifp = &ethdev->eth_if;

	/* 广播地址以及多播地址可以直接根据目的IP得到 */
	if(m->m_flags & MF_BCAST) {
		bcopy(ether_broadaddr, macaddr, ETH_ADDR_LEN);
		return 0;
	}
	if(m->m_flags & MF_MCAST) {
		ETHER_MAP_IP_MUILTCAST(&satosin(dst)->sin_addr.s_addr, macaddr);
		return 0;
	}
	
	/* 获取链路信息 */
	if(rt != NULL && (rt->rt_flags & RTF_LLINFO)) 
		la = (struct llinfo_arp *)rt->rt_llinfo;
	else  {
		if((la = arplookup(satosin(dst)->sin_addr.s_addr, 1)) != NULL)
			rt = la->la_rt;
	}
	if(!rt || !la) {
		log(LOG_ERR, "arpresolve: can't find any link info for %s\n", 
			inet_ntoa(satosin(dst)->sin_addr));
		mbuf_freem(m);
		return -1;
	}

	sdl = satosdl(rt->rt_gateway);

	/* 检查当前ARP中的ARP信息是否有效 */
	if((!rt->rt_expire) && (AF_LINK == sdl->sdl_family) && 
			(sdl->sdl_alen != 0)) {
		bcopy(LLADDR(sdl), macaddr, sdl->sdl_alen);
		return 0;
	}

	if(ifp->if_flags & IFF_NOARP) {
		log(LOG_ERR, "arpresolve: ifnet don't support arp\n");
		mbuf_freem(m);
		return -1;
	}

	/* 保存最后一次进行ARP地址解析的数据报文 */
	if(la->la_hold)
		mbuf_freem(la->la_hold);
	la->la_hold = m;
		
	/* bad code 临时代码，后续需要加上请求重传次数以及挂起路由的处理代码 */
	if(rt->rt_expire) {
		rt->rt_flags &= ~RTF_REJECT;
		arprequest(ethdev, &satosin(rt->rt_ifa->ifa_addr)->sin_addr.s_addr, 
			&satosin(dst)->sin_addr.s_addr, ethdev->eth_addr);
	}

	return -1;
}

/**
 * 描述：根据IP地址查找ARP表（带有链路信息的专属路由）
 * 返回：返回找到的链路信息
 */
struct llinfo_arp *arplookup(in_addr_t addr, int report)
{
	struct sockaddr_in sin = {0};//注意这里要将sin清零
	struct rtentry *rt;

	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(struct sockaddr_in);
	sin.sin_addr.s_addr = addr;
	
	if(!(rt = __rt_alloc(sintosa(&sin), report)))
		return NULL;
	
	if((rt->rt_flags & RTF_GATEWAY) || !(rt->rt_flags & RTF_LLINFO) 
			|| (rt->rt_gateway->sa_family != AF_LINK) 
			|| (rt->rt_gateway->sa_len < sizeof(struct sockaddr_dl)))
		return NULL;

	return (struct llinfo_arp *)rt->rt_llinfo;
}

/**
 *  描述：ARP初始化函数，该函数将在设置一个接口的IP地址时被调用
 */
void arp_ifinit(struct ether_dev *ethdev, struct ifaddr *ifa)
{
	/* 免费ARP报文调整到第一次检测到物理链接的时候发送 */
#if 0
	/* 发送一条free arp报文，用于判断是否有IP地址冲突，此外还可以更新对方的ARP表中保存的我们的地址信息 */
	arprequest(ethdev, &satosin(ifa->ifa_addr)->sin_addr.s_addr, 
		&satosin(ifa->ifa_addr)->sin_addr.s_addr, ethdev->eth_addr);
#endif
	ifa->ifa_rtrequest = arp_rtrequest;
}

/**
 * 描述：将传入MAC地址数组转换成可打印的格式字符串
 * 注意：由于使用的是静态缓冲区，因此在printf函数中调用多次该函数时，打印的值将会是一样的
 * 		 ，与此类似的函数还有inet_ntoa
 */
static char *ether_mactostr(const uint8_t *macaddr)
{
	int i;
	static char *digits = "0123456789abcdef";
	static char macbuf[3 * ETH_ADDR_LEN];
	char *cp = macbuf;

	for(i = 0; i < ETH_ADDR_LEN; ++i) {
		*cp++ = digits[macaddr[i] >> 4];
		*cp++ = digits[macaddr[i] & 0x0f];
		*cp++ = ':';
	}
	*--cp = 0;

	return macbuf;
}
