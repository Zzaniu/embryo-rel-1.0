
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * internet 协议控制块相关函数 
 * 
 */
#include <common.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/list.h>
#include <sys/system.h>
#include <sys/endian.h>					
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/protocol.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <mach/intr.h>

static in_port_t ipport_firstauto = IPPORT_RESERVED;
static in_port_t ipport_lastauto = IPPORT_USERRESERVED;
static in_port_t ipport_hifirstauto = IPPORT_HIFIRSTAUTO;
static in_port_t ipport_hilastauto = IPPORT_HILASTAUTO;

/* inpcb哈希函数 */
#define INPCBHASH(table, faddr, fport, laddr, lport)	\
		&(table)->int_pcbhashtbl[(ntohl((faddr)->s_addr) + \
		ntohs(fport) + ntohl((laddr)->s_addr) + ntohs(lport)) & (table)->int_hashmask]

/* 判断端口是否合法 */
static int in_badport(in_port_t port)
{
	/* 当前不支持预留端口 */
	if(port <= IPPORT_RESERVED)
		return 1;
	return 0;
}

/**
 * 描述：初始化inpcbtable
 * 参数：table，需要初始化的inpcb表；hashsize，表示inpcb哈希表的数据大小（桩数），
 * 		 注意，该数必须为2的幂，否则实际桩大小将小于指定的hashsize大小
 */
void init_inpcb(struct inpcbtable *table, ulong hashsize)
{
	INIT_LIST_HEAD(&table->int_pcbqueue);
	
	table->int_pcbhashtbl = hashinit(hashsize, &table->int_hashmask);
	table->int_lastport = 0;
}

/**
 * 描述：分配一个in_pcb并且将该结构插入到全局的table中，并建立与socket的关联
 */
int alloc_inpcb(struct socket *so, struct inpcbtable *tlbp)
{
	int s;
	struct in_pcb *inp;

	if(NULL == (inp = (struct in_pcb *)kmem_malloc(sizeof(struct in_pcb)))) 
		return ENOMEM;
	bzero(inp, sizeof(*inp));//inp_faddr = inp_laddr = INADDR_ANY

	inp->inp_sock = so;
	inp->inp_tlb = tlbp;

	/* 将in_pcb插入到table的队列以及table中的哈希表中 */
	s = splnet();
	list_add(&inp->inp_queue, &tlbp->int_pcbqueue); 
	/* 真的很怀疑这里哈希表起到的作用。。。 */
	list_add(&inp->inp_hash, INPCBHASH(tlbp, &inp->inp_faddr, inp->inp_fport,
			&inp->inp_laddr, inp->inp_lport));
	splx(s);

	so->soc_pcb = inp;

	return 0;
}

/**
 * 描述：释放in_pcb资源
 */
void free_inpcb(struct socket *so)
{
	int s;
	struct in_pcb *inp = so->soc_pcb;

	so->soc_pcb = NULL;

	/* 释放路由以及选项资源 */
	if(inp->inp_option)
		mbuf_freem(inp->inp_option);
	if(inp->inp_ro.ro_rt)
		;//rtfree(inp->inp_ro.ro_rt);//bad code

	/* 从哈希表以及队列中将当前inp移除 */
	s = splnet();
	list_del(&inp->inp_hash);
	list_del(&inp->inp_queue); 
	splx(s);

	kmem_free(inp);
}

/**
 * 描述：绑定本地地址对，当传入的端口号为0时或addr为NULL时，将分配一个新的可用端口
 * 		 本函数将检查地址占用的情况，当socket没有设置SO_REUSEPORT选项，并且出现本地
 * 		 地址匹配（当设置了SO_REUSEADDR时，要求绝配，否则仅要求通陪）的情况时，将出
 * 		 现地址占用
 * 参数：addr为本地地址+端口
 */ 
int inpcb_bind(struct in_pcb *inp, struct mbuf *addr)
{
	int flags, portreuse;
	struct sockaddr_in *laddr;
	in_port_t lport = 0, first, last;
	struct in_addr zero_addr = {0};
	struct in_pcb *t;
	struct socket *so = inp->inp_sock;

	/* bind函数要求必须有本地接口配置了IP地址 */
	if(list_empty(&in_ifaddrlist))
		return EADDRNOTAVAIL;

	/* bind之前，不能存在已经绑定的地址或端口，即bind仅能调用一次 */
	if(inp->inp_laddr.s_addr != INADDR_ANY || inp->inp_lport)
		return EINVAL;

	/* 根据socket是否设置了地址与端口重用来决定是进行绝配查找还是进行通配查找 */
	if(!(so->soc_options & (SO_REUSEADDR | SO_REUSEPORT | SO_ACCEPTCONN)) &&
				!(so->soc_proto->pr_flags & PRF_CONNREQUIRED))
		flags = INPCBLOOKUP_WILDCARD;
	portreuse = so->soc_options & SO_REUSEPORT;

	/* 当addr不为NULL时，进行地址绑定，该过程将检查地址是否重用 */
	if(addr) {
		laddr = mtod(addr, struct sockaddr_in *);
		if(laddr->sin_len != sizeof(struct sockaddr_in))
			return EINVAL;
		if(laddr->sin_family != AF_INET)
			return EPROTONOSUPPORT;
		lport = laddr->sin_port;
		if(IN_MULTICAST(laddr->sin_addr.s_addr)) {
			/* 暂不支持多播地址 */
			return EADDRNOTAVAIL;
		} else if(laddr->sin_addr.s_addr != INADDR_ANY) {
			/* 进行本地地址查找时，由于设置主机地址时不带端口号，因此这里必须将此字段清0，否则将查找失败 */
			laddr->sin_port = 0;
			/* 如果要绑定的地址不在本地IP链表中，将出错 */
			if(!ifa_ifwithaddr(sintosa(laddr)))
				return EADDRNOTAVAIL;
		}
		/* 当同时指定端口以及地址时，需要检查重用性 */
		if(lport) {	
			/* 检查端口是否合法 */
			if(in_badport(ntohs(lport)))
				return EINVAL;
			/* 当设置原有的地址对和当前地址对均设置了SO_REUSEPORT选项时，将忽略本地地址重用性检查 */	
			if(((t = inpcb_lookup(inp->inp_tlb, flags, zero_addr, 0,
					laddr->sin_addr, lport)) != NULL) && 
					!(portreuse & t->inp_sock->soc_options))
				return EADDRINUSE;
		}
		/* 绑定本地地址 */
		inp->inp_laddr = laddr->sin_addr;
	}

	/* 如果本地端口为0，将自动分配一个新端口号 */
	if(!lport) {
		if(inp->inp_flags & INP_HIPORT) {
			first = ipport_hifirstauto;
			last = ipport_hilastauto;
		} else {
			first = ipport_firstauto;
			last = ipport_lastauto;
		}
		lport = first;
		do {
			/* 找到一个未被使用的端口 */
			if(!inpcb_lookup(inp->inp_tlb, flags, zero_addr, 0,
					inp->inp_laddr, htons(lport)))
				break;
		} while(++lport <= last);
		if(lport > last) {
			/* 清除上面绑定的本地地址 */
			inp->inp_laddr.s_addr = INADDR_ANY;
			return EADDRNOTAVAIL;
		}
		inp->inp_tlb->int_lastport = lport;
		/* 注意lport字节序 */
		lport = htons(lport);
	}
	/* 绑定本地端口 */
	inp->inp_lport = lport;
	inpcb_rehash(inp);
	
	return 0;
}

/**
 * 描述：建立本地-远端地址对的绑定关系
 * 		 本函数将检查地址占用的情况，当本地-远端地址出现绝对匹配时，将出现地址占用
 * 参数：addr为远端地址+端口
 */
int inpcb_connect(struct in_pcb *inp, struct mbuf *addr)
{
	struct sockaddr_in *faddr, *sin;
	struct in_addr laddr;
	struct in_ifaddr *ia; 
	struct route *ro;
	struct socket *so = inp->inp_sock;
	in_port_t fport, lport;

	/* 检查远端地址有效性 */
	if(!addr)
		return EINVAL;
	faddr = mtod(addr, struct sockaddr_in *);
	if(faddr->sin_len != sizeof(struct sockaddr_in))
		return EINVAL;
	if(faddr->sin_family != AF_INET)
		return EPROTONOSUPPORT;
#if 0
	/* 这里对远端的端口号不进行校验 */
	if(in_badport(ntohs(faddr->sin_port)))
		return EADDRNOTAVAIL;
#endif
	if(IN_MULTICAST(faddr->sin_addr.s_addr)) {
		/* 暂不支持多播地址 */
		return EADDRNOTAVAIL;
	}
	/* 没有任何本地地址 */
	if(list_empty(&in_ifaddrlist)) 
		return EADDRNOTAVAIL;
	
	/* 处理特殊地址的情况 */
	ia = list_first_entry(&in_ifaddrlist, struct in_ifaddr, ia_list);
	/* 当目的地址为ANY时，将视为主接口地址 */
	if(INADDR_ANY == faddr->sin_addr.s_addr) 
		faddr->sin_addr = ia->ia_addr.sin_addr;
	/* 当目的地址为全f的广播地址时，将视为主接口对应广播地址 */
	if(INADDR_BROADCAST == faddr->sin_addr.s_addr &&
				(ia->ia_ifp->if_flags & IFF_BROADCAST))  
		faddr->sin_addr = ia->ia_broadaddr.sin_addr;

	lport = inp->inp_lport;
	laddr = inp->inp_laddr;
	/* 当为绑定本地地址时，将根据远端地址确定本地地址（可能还包括路由） */
	if(INADDR_ANY == inp->inp_laddr.s_addr) {
		ro = &inp->inp_ro;
		if(ro->ro_rt && (((satosin(&ro->ro_dst))->sin_addr.s_addr != faddr->sin_addr.s_addr) 
				|| (so->soc_options & SO_DONTROUTE))) {
			/* 如果当前目的地址与之前记录的路由目的地址不同，或者设置了不是用路由选项，则清楚老的路由信息 */
			;//rtfree(ro->ro_rt) bad code
			ro->ro_rt = NULL;	
		}
		/* 这里将分配一个到达目的地址的路由给inp，下次调用如果目的地址一样，将不用重新查找路由 */
		if(!(so->soc_options & SO_DONTROUTE) && (!ro->ro_rt || !ro->ro_rt->rt_ifp)) {
			sin = satosin(&ro->ro_dst);
			/* 注意：这里必须将port\zero字段清0，否则路由匹配时可能找不到对应路由 */
			bzero(sin, sizeof(struct sockaddr_in));
			sin->sin_family = AF_INET;
			sin->sin_len = sizeof(struct sockaddr_in);
			sin->sin_addr = faddr->sin_addr;
			rt_alloc(ro);
		}
		/* 定位本机地址 */
		if(ro->ro_rt)
			ia = ifatoia(ro->ro_rt->rt_ifa);
		else {
			/* 如果没有找到路由，而又没设置不使用路由选项，ip_output函数将报错 */
			fport = faddr->sin_port;
			faddr->sin_port = 0;//注意：这里必须清0，否则将会引起下面的地址查找失败
			ia = ifatoia(ifa_ifwithdstaddr(sintosa(faddr)));		//查找匹配P2P端口的本地地址
			if(!ia)
				ia = ifatoia(ifa_ifwithnet(sintosa(faddr)));		//查找匹配网络的本地地址
			if(!ia)
				ia = list_first_entry(&in_ifaddrlist, struct in_ifaddr, ia_list);	//使用本地主接口地址
			faddr->sin_port = fport;
		}
		laddr = ia->ia_addr.sin_addr;
		/* 如果没有bind本地端口，调用bind自动分配一个可用端口 */
		if(!inp->inp_lport) {
			if(inpcb_bind(inp, NULL))
				return EADDRNOTAVAIL;
			lport = inp->inp_lport;//保存新分配到的端口
			inp->inp_lport = 0;	//避免后面出错引起inp误设置
		}
	}

	/* 检查是否有本地-远端地址对的绝对匹配 */
	if(inpcb_hashlookup(inp->inp_tlb, 
			faddr->sin_addr, faddr->sin_port,
			laddr, lport) != NULL)
		return EADDRNOTAVAIL;

	/* 本地-远端地址对绑定 */
	inp->inp_lport = lport;
	inp->inp_laddr = laddr;
	inp->inp_fport = faddr->sin_port;
	inp->inp_faddr = faddr->sin_addr;

	inpcb_rehash(inp);

	return 0;
}

/**
 * 描述：解除与远端地址对之间的绑定关系
 */
void inpcb_disconnect(struct in_pcb *inp)
{
	inp->inp_faddr.s_addr = INADDR_ANY;
	inp->inp_fport = 0;
	inpcb_rehash(inp);
}

/**
 * 描述：当inp中的本地-远端地址对进行了修改，需要调用此函数将in_pcb插入到哈希表新的位置
 */
void inpcb_rehash(struct in_pcb *inp)
{
	/* 从原先位置移除*/
	list_del(&inp->inp_hash);
	/* 重新插入新位置 */
	list_add(&inp->inp_hash, INPCBHASH(inp->inp_tlb, &inp->inp_faddr, inp->inp_fport,
			&inp->inp_laddr, inp->inp_lport));
}

/**
 * 描述：hash查找in_pcb
 */
struct in_pcb *inpcb_hashlookup(struct inpcbtable *table, 
			struct in_addr faddr, uint16_t fport,
			struct in_addr laddr, uint16_t lport)
{
	struct list_head *head;
	struct in_pcb *cur;	
	
	head = INPCBHASH(table, &faddr, fport, &laddr, lport);
	
	list_for_each_entry(cur, head, inp_hash) {	
		if(cur->inp_faddr.s_addr == faddr.s_addr &&
				cur->inp_fport == cur->inp_lport &&
				cur->inp_laddr.s_addr == laddr.s_addr &&
				cur->inp_lport == cur->inp_lport) {
			/* 如果找到了对应元素，将当前元素提升到对首位置，以提高下次访问速度 */
			list_del(&cur->inp_hash);
			list_add(&cur->inp_hash, INPCBHASH(table, &faddr, fport,
					&laddr, lport));
			return cur;
		}
	}

	return NULL;
}

/**
 * 描述：通陪查找in_pcb，如果远端和本地地址中有任何一个为INADDR_ANY且另一个不为INADDR_ANY
 * 		 则认为是一次通陪。该函数返回通配最少的最优项
 */ 
struct in_pcb *inpcb_lookup(struct inpcbtable *table, int flags,
			struct in_addr faddr, uint16_t fport,
			struct in_addr laddr, uint16_t lport)
{
	int wild, min_wild = 3;
	struct in_pcb *cur, *best = NULL;	
	
	list_for_each_entry(cur, &table->int_pcbqueue, inp_queue) {	
		/* 要求本地端口绝对匹配 */
		if(cur->inp_lport != lport)
			continue;
		wild = 0;
		if(cur->inp_laddr.s_addr != INADDR_ANY) {
			if(INADDR_ANY == laddr.s_addr)
				++wild;
			else if(cur->inp_laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if(laddr.s_addr != INADDR_ANY)
				++wild;
			/* 当本地和远端地址均为INADDR_ANY时，将认为是绝对匹配 */
		}
		if(cur->inp_faddr.s_addr != INADDR_ANY) {
			if(INADDR_ANY == faddr.s_addr)
				++wild;
			/* 注意进行远端端口匹配时，如果本地远端中有任何一个地址为INADDR_ANY，则忽略端口是否匹配 */
			else if(cur->inp_faddr.s_addr != faddr.s_addr || cur->inp_fport != fport)
				continue;
		} else {
			if(faddr.s_addr != INADDR_ANY)
				++wild;
		}
		/* 寻找最接近的本地-远端地址对匹配 */
		if((!wild || (flags & INPCBLOOKUP_WILDCARD)) && wild < min_wild) {
			/* 如果是绝对匹配则直接返回当前找到的in_pcb */
			if(!(min_wild = wild))
				return cur;
			best = cur;
		}
	}

	return best;
}

