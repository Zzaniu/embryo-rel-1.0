
/**
 * Copyright(c) 2017-8-17 Shangwen Wu	
 *
 * IP协议层相关函数
 * 
 */
#include <common.h>
#include <stdarg.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/endian.h>					
#include <sys/system.h>
#include <sys/param.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protocol.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>

static ulong ipqmaxlen = IFQ_MAXLEN;
static int ip_dosrcroute = 0;	//IP是否支持源站选路
static int ip_forwarding = 0;	//是否支持IP曾转发

/* IP协议层输入队列 */
struct ifqueue ipintrq;

static struct sockaddr_in ipaddr = {sizeof(ipaddr), AF_INET};
static struct route ipforward_rt = {};
static int ip_nhops = 0;

/* ，数组索引为AF_XXX（协议编码），数组值为对应协议结构体在inet_protos的索引 */
uchar ip_protox[IPPROTO_MAX];	//IP层转发表

extern struct protocol inet_protos[];	//defined in in_proto.c
extern struct domain inetdomain;

/**
 * 描述：初始化全局IP协议族转发表
 */
void ip_init(void)
{
	int i;
	struct protocol *pr;

	pr = profindbyproto(AF_INET, SOCK_RAW, IPPROTO_RAW); 
	if(NULL == pr)
		panic("ip_init");
	
	/* 默认所有协议均指向原始数据协议 */
	for(i = 0; i < IPPROTO_MAX; ++i)
		ip_protox[i] = pr - inet_protos;

	for(pr = inetdomain.dom_proto; pr < inetdomain.dom_proto_last; ++pr) 
		if(pr->pr_protocol && pr->pr_protocol != IPPROTO_RAW)
			ip_protox[pr->pr_protocol] = pr - inet_protos;

	ipintrq.ifq_maxlen = ipqmaxlen;
}

/**
 * 描述：IP报文软件中断处理程序
 */
void ipintr(void)
{
	int s;
	struct mbuf *m;

	IPLVL_DBG("In ipintr\n");

	while(1) {
		s = splimp();
		IFQ_DEQUEUE(&ipintrq, m);
		splx(s);
		if(!m)
			return;
		ipv4_input(m);
	}
}

/**
 * 描述：IP层入口
 * 注意：调用该函数后，m所在的空间将被释放
 */
int ipv4_input(struct mbuf *m, ...)
{
	struct ip *ip;
	struct in_ifaddr *ia;
	ulong hlen;

	IPLVL_DBG("In ipv4_input\n");

	/* 如果当前主机无任何IP地址，数据包将直接被丢弃 */
	if(list_empty(&in_ifaddrlist)) 
		goto fail;

	/* 如果第一个mbuf的长度小于IP头长度，则丢弃数据包 */
	if(m->m_len < sizeof(struct ip) &&
		!(m = mbuf_pullup(m, sizeof(struct ip))))
		goto fail;

	ip = mtod(m, struct ip *);
	/* 如果收到数据报文不是IPV4，则丢弃数据包 */
	if(ip->ip_ver != IPVERSION)
		goto fail;

	/* ip包头长度非法 */
	hlen = ip->ip_hlen << 2;
	if(hlen < sizeof(sizeof(struct ip)))
		goto fail;

	/* 将第一个mbuf的有效长度设置成IP包头总长 */
	if(hlen > m->m_len && !(m = mbuf_pullup(m, hlen)))
		return -1;

	ip = mtod(m, struct ip *);
	/* 校验和出错*/
	if((ip->ip_sum = in_cksum(m, hlen)) != 0)
		goto fail;
	
	ip->ip_len = ntohs(ip->ip_len);
	/* ip总长度非法 */
	if(ip->ip_len < hlen)
		goto fail;
	
	ip->ip_id = ntohs(ip->ip_id);
	ip->ip_off = ntohs(ip->ip_off);

	/* pkthdr长度非法 */
	if(m->m_pkthdr.mp_len < ip->ip_len)
		goto fail;

	/* 调整mbuf中有效数据的长度 */
	if(m->m_pkthdr.mp_len > ip->ip_len) {
		if(m->m_pkthdr.mp_len == m->m_len) 
			m->m_pkthdr.mp_len = m->m_len = ip->ip_len;
		else 
			mbuf_trim(m, m->m_pkthdr.mp_len - ip->ip_len, 1);
	}

	/* 处理IP选项 */
	ip_nhops = 0;//清空当前反向路由个数
	if(hlen > sizeof(struct ip) && ip_dooptions(m))
		goto fail;

	/* 判断IP目的地址是否为本机地址 */
	if((ia = in_iawithaddr(ip->ip_dst, m)) != NULL && 
			(ia->ia_ifp->if_flags & IFF_UP))
		goto ours;

	/* 判断是否为多播地址 */
	if(IN_MULTICAST(ip->ip_dst.s_addr)) {
		/* 查询当前接收接口是否位于该地址所在的多播组 */
		//goto ours;
	}

	/* 广播地址 */
	if(INADDR_ANY == ip->ip_dst.s_addr ||
			INADDR_BROADCAST == ip->ip_dst.s_addr)
		goto ours;

	/* 不属于本机的数据报文将被转发 */
	if(!ip_forwarding)
		mbuf_freem(m);
	else 
		ip_forward(m, 0);	

	return 0;

/* 本机需要处理的数据报文 */
ours:
	/* 注意：ip_len长度为IP包总长减去IP头长 */
	ip->ip_len -= hlen;
	
	/* 调用上一层协议的input函数 */
	(*inet_protos[ip_protox[ip->ip_pro]].pr_input)(m, hlen, NULL, 0);

	return 0;

fail:
	log(LOG_ERR, "Ip_input fail\n");

	mbuf_freem(m);

	return -1;
}

/**
 * 查找本机匹配的IP地址
 */
struct in_ifaddr *in_iawithaddr(struct in_addr ina, struct mbuf *m)
{
	struct in_ifaddr *ia;

	list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
		/* 查找是否有接口地址匹配 */
		if((ina.s_addr == ia->ia_addr.sin_addr.s_addr) ||
				(INADDR_ANY == ia->ia_addr.sin_addr.s_addr))
			return ia;
		/* 查找是否有广播地址匹配 */
		if(m && (ia->ia_ifp == m->m_pkthdr.mp_recvif) &&
				(ia->ia_ifp->if_flags & IFF_BROADCAST)) {
			if(ina.s_addr == ia->ia_broadaddr.sin_addr.s_addr || 
					ina.s_addr == ia->ia_netbroadcast.s_addr ||
					ina.s_addr == ia->ia_subnet ||
					ina.s_addr == ia->ia_net)
				return ia;
		}
	}

	return NULL;
}

/**
 * 描述：根据IP地址查找合适的转发路由
 */
struct in_ifaddr *ip_rtaddr(struct in_addr dst)
{
	struct sockaddr_in *sin = satosin(&ipforward_rt.ro_dst);

	/* 当前该地址的路由为空，待查找 */
	if(!ipforward_rt.ro_rt && dst.s_addr != sin->sin_addr.s_addr) {
		if(ipforward_rt.ro_rt) {
			//rt_free(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = NULL;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(sin);
		sin->sin_addr = dst;
		//查找路由
		rt_alloc(&ipforward_rt);
	}

	if(!ipforward_rt.ro_rt)		//没有对应路由
		return NULL;
	
	return ifatoia(ipforward_rt.ro_rt->rt_ifa);	
}

/**
 * 保存源站地址接口，以便IP曾提供反向路由
 */
static struct ip_srcrt {
	struct in_addr ips_dst;		//反向路由的第一个目的地址
	uint8_t ips_nop;			//对齐用
	uint8_t ips_srcopt[3];		//code,len,ptr
	struct in_addr ips_route[MAXIPOPTLEN / sizeof(struct in_addr)];//反向路由
} ip_srcrt;

/**
 * 描述：保存源站路由信息
 */
static void save_rte(caddr_t option, struct in_addr dst)
{
	int olen;

	olen = option[IPOPT_LEN];

	if(olen > sizeof(ip_srcrt) - (1 + sizeof(dst)))
		return;

	ip_srcrt.ips_dst = dst;
	bcopy(option, (caddr_t)ip_srcrt.ips_srcopt, olen);
	ip_nhops = (olen - IPOPT_PTR - 1) / sizeof(struct in_addr);
}

/**
 * 描述：计算源站路由的反向路由，并且将其保存到一个mbuf空间
 * 返回：保存反向路由的mbuf指针，返回空表示当前接收到的ip报文没有添加选站路由选项
 */
struct mbuf *ip_srcroute(void)
{
	struct in_addr *p, *q;
	struct mbuf *m;
	ulong optsz = sizeof(ip_srcrt.ips_nop) + sizeof(ip_srcrt.ips_srcopt);

	if(!ip_nhops)
		return NULL;
	if(NULL == (m = mbuf_get(MT_SOOPTS)))
		return NULL;

	m->m_len = sizeof(struct in_addr) * (ip_nhops + 1) + optsz;
	p = &ip_srcrt.ips_route[ip_nhops - 1];
	*(mtod(m, struct in_addr *)) = *p--;
	ip_srcrt.ips_nop = IPOPT_NOP;
	ip_srcrt.ips_srcopt[IPOPT_PTR] = IPOPT_MINOFF;
	bcopy((caddr_t)&ip_srcrt.ips_nop, mtod(m, caddr_t) + sizeof(struct in_addr), optsz);
	q = (struct in_addr *)(mtod(m, caddr_t) + sizeof(struct in_addr) + optsz);
	
	while(p >= ip_srcrt.ips_route)
		*q++ = *p--;

	*q = ip_srcrt.ips_dst;
	
	return m;
}

/**
 * 描述：处理IP协议选项（时间戳、源站路由、路由记录）
 * 注意：处理IP选项时出错将发送一个ICMP报文
 */
int ip_dooptions(struct mbuf *m)
{
	struct ip *ip = mtod(m, struct ip *);
	caddr_t cp = (caddr_t)(ip + 1);
	int code, type = ICMP_PARAMPROB;
	int cnt = (ip->ip_len << 2) - sizeof(ip), olen = 0, opt, off, forward;
	struct in_ifaddr *ia;
	struct ip_timestamp *ipt;
	n_time ntime;
	
	for(; cnt > 0; cnt -= olen, cp += olen) {
		opt = cp[IPOPT_CODE];
		if(IPOPT_EOL == opt)
			break;
		if(IPOPT_NOP == opt) {
			olen = 1;
			continue;
		} else {
			olen = cp[IPOPT_LEN];
			if(olen > cnt || olen <= 0) {
				code = &cp[IPOPT_LEN] - (caddr_t)ip;
				goto bad;
			}
		}
		switch(opt) {
			case IPOPT_SSRR:
			case IPOPT_LSRR:
				if(!ip_dosrcroute) {	//不支持源站选路
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					log(LOG_WARNING, "Attempted source route!\n");
					goto bad;
				}
				off = cp[IPOPT_PTR];
				if(off < IPOPT_MINOFF) {
					code = &cp[IPOPT_PTR] - (caddr_t)ip;
					goto bad;
				}
				ipaddr.sin_addr = ip->ip_dst;
				/* 查找入口地址是否位于本机 */			
				if(!(ia = ifatoia(ifa_ifwithaddr(sintosa(&ipaddr))))) {
					if(IPOPT_SSRR == opt) {		//如果为严格路由，而目的地址不是本机，将返回icmp差错
						type = ICMP_UNREACH;
						code = ICMP_UNREACH_SRCFAIL;
						goto bad;
					} else
						break;			//如果为宽松路由，则将其作为普通报文转发
				}
				off--;
				/* 当前主机属于最终的目的地址 */
				if(off > olen - sizeof(struct in_addr)) {
					save_rte(cp, ip->ip_src);
					break;
				}
				/* 定位出口路由，选项的off位置保存下一跳目的地址  */
				bcopy(cp + off, (caddr_t)&ipaddr.sin_addr, sizeof(struct in_addr));
				if(IPOPT_SSRR == opt) {	//严格路由查询与本机接口直接相联的网络
					if(!(ia = ifatoia(ifa_ifwithdstaddr(sintosa(&ipaddr))))) 
						ia = ifatoia(ifa_ifwithnet(sintosa(&ipaddr)));
				} else {		//宽松路由查询当前主机路由表匹配的（包括直接和间接）网络
					ia = ip_rtaddr(ipaddr.sin_addr);
				}
				if(NULL == ia) {//找不到出口路由
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				}
				/* 修改IP报文的目的地址为下一跳地址 */	
				ip->ip_dst = ipaddr.sin_addr;
				/* 将出口地址替换到原来的下一跳路由地址（即off）位置 */
				bcopy((caddr_t)&ia->ia_addr.sin_addr, cp + off, sizeof(struct in_addr));
				/* 选项指针移动到下一个地址 */
				cp[IPOPT_PTR] += sizeof(struct in_addr);
				/* 查询目的地址是否为广播地址，如果为广播地址，将不会进行装发 */
				forward = !IN_MULTICAST(ip->ip_dst.s_addr);
				break;
			case IPOPT_RR:
				off = cp[IPOPT_PTR];
				if(off < IPOPT_MINOFF) {
					code = &cp[IPOPT_PTR] - (caddr_t)ip;
					goto bad;
				}
				off--;
				if(off > olen - sizeof(struct in_addr))
					break;						//空间不够时，不做任何处理，直接跳过该选项
				/* 查找出口地址 */
				bcopy((caddr_t)&ip->ip_dst, (caddr_t)&ipaddr.sin_addr, sizeof(struct in_addr));
				if(!(ia = ifatoia(ifa_ifwithaddr(sintosa(&ipaddr)))) || \
						!(ia = ip_rtaddr(ipaddr.sin_addr))) {
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_HOST;
					goto bad;
				}
				/* 将本机的出口地址拷贝到off指向位置 */
				bcopy((caddr_t)&ia->ia_addr.sin_addr, cp + off, sizeof(struct in_addr));				
				/* 选项指针移动到下一个存储位置 */
				cp[IPOPT_PTR] += sizeof(struct in_addr);
				break;
			case IPOPT_TS:
				code = &cp[IPOPT_CODE] - (caddr_t)ip;
				ipt = (struct ip_timestamp *)cp;
				if(ipt->ipt_ptr < 5 || ipt->ipt_len < 5)
					goto bad;
				if(ipt->ipt_ptr - 1 > ipt->ipt_len - sizeof(n_time)) {
					if(++ipt->ipt_oflw == 0) //发生溢出
						goto bad;
					break;
				}
				switch(ipt->ipt_flg) {
					case IPOPT_TS_TSONLY:
						break;
					case IPOPT_TS_TSANDADDR:
						if(ipt->ipt_ptr - 1 > ipt->ipt_len - sizeof(n_time) - sizeof(struct in_addr)) 
							goto bad;
						ipaddr.sin_addr = ip->ip_dst;
						if(!(ia = ifatoia(ifaof_ifpforaddr(sintosa(&ipaddr), m->m_pkthdr.mp_recvif))))
							continue;
						bcopy((caddr_t)&ia->ia_addr.sin_addr, cp + ipt->ipt_ptr - 1, sizeof(struct in_addr));
						ipt->ipt_ptr += sizeof(struct in_addr);
						break;
					case IPOPT_TS_PRESPEC:
						if(ipt->ipt_ptr - 1 > ipt->ipt_len - sizeof(n_time) - sizeof(struct in_addr)) 
							goto bad;
						bcopy(cp + ipt->ipt_ptr - 1, (caddr_t)&ipaddr.sin_addr, sizeof(struct in_addr));
						if(!(ia = ifatoia(ifa_ifwithaddr(sintosa(&ipaddr)))))
							continue;
						ipt->ipt_ptr += sizeof(struct in_addr);
						break;
					default:
						goto bad;
				}
				ntime = iptime();
				bcopy((caddr_t)&ntime, cp + ipt->ipt_ptr - 1, sizeof(n_time));
				ipt->ipt_ptr += sizeof(n_time);
				break;
			default:
				goto bad;
		}
	}

	if(forward && ip_forwarding) {
		ip_forward(m, 1);
		return 1;
	}

	return 0;

bad:
	ip->ip_len -= ip->ip_hlen << 2;	

	icmp_error(m, type, code, 0, NULL);

	return 1;
}

/**
 * 描述：IP曾转发函数
 * 参数：m，转发数据缓冲区，srcrt是否进行源站路由
 */
void ip_forward(struct mbuf *m, int srcrt)
{
	//BIOS暂时未实现
	mbuf_freem(m);
}

/**
 * 描述：剥离选项数据
 * 参数：如果mo不为NULL，则将分离的选项数据拷贝到mo中
 */
void ip_stripoptions(struct mbuf *m, struct mbuf *mo)
{
	ulong hlen, olen;
	struct ip *ip = mtod(m, struct ip *);

	hlen = ip->ip_hlen << 2;
	if(hlen <= sizeof(struct ip))
		return;
	olen = hlen - sizeof(struct ip);
	
	if(mo) {
		bcopy((caddr_t)(ip + 1), mo->m_data, olen);
		mo->m_len = olen;
	}
	
	bcopy((caddr_t)(ip + 1) + olen, (caddr_t)(ip + 1), m->m_len - hlen);
	
	ip->ip_hlen = sizeof(struct ip) >> 2;
	ip->ip_len -= olen;
	m->m_len -= olen;
	if(m->m_flags & MF_PKTHDR)
		m->m_pkthdr.mp_len -= olen;
}
