
/**
 * Copyright(c) 2017-7-25 Shangwen Wu	
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
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protocol.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip_var.h>
#include <netinet/ip.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>

static struct mbuf *ip_insertoptions(struct mbuf *m, struct mbuf *mo);

/**
 * 描述：IP层出口函数
 * 参数：m，保存上层协议数据的缓冲区, opt，保存上层选项数据，
 * 		 ro，保存上层指定的路由，flags，标识位
 * 注意：m中的IP数据不能包含选项，上层必须将IP头中的选项和IP头公共字段分离；
 * 		 该函数要求传入的ip头中，ip_len、ip_off、ip_id这三个字段的字节序为主机字节序
 */
int ip_output(struct mbuf *m, ...)
{
	va_list ap;
	struct ip *ip;
	int flags, error = 0;
	struct mbuf *opt;
	int hlen;
	struct sockaddr_in *dst;
	struct ifnet *ifp;
	struct in_ifaddr *ia;
	struct route *ro;
	struct route iproute;

	va_start(ap, m);
	opt = va_arg(ap, struct mbuf *);
	ro = va_arg(ap, struct route *);
	flags = va_arg(ap, int);
	va_end(ap);

	if(!(m->m_flags & MF_PKTHDR))
		panic("ip_output mbuf is not pkthdr");

	if(opt) 
		m = ip_insertoptions(m, opt);
	
	ip = mtod(m, struct ip *);
	if(!(flags & (IP_FORWARDING | IP_RAWOUTPUT))) {
		ip->ip_ver = IPVERSION;
		ip->ip_id = 0;
		ip->ip_id = htons(ip->ip_id);
	}
	hlen = ip->ip_hlen << 2;

	if(NULL == ro) {
		ro = &iproute;
		bzero((caddr_t)ro, sizeof(struct route));
	}

	dst = satosin(&ro->ro_dst);

	/* 搜索目的地址的路由 */
	if(ro->ro_rt && (!(ro->ro_rt->rt_flags & RTF_UP) \
			|| dst->sin_addr.s_addr != ip->ip_dst.s_addr)) {
		//rt_free(ro->ro_rt);
		ro->ro_rt = NULL;
	}

	if(NULL == ro->ro_rt) {
		dst->sin_family = AF_INET;
		dst->sin_len = sizeof(struct sockaddr_in);
		dst->sin_addr.s_addr = ip->ip_dst.s_addr;
	}

	if(flags & IP_ROUTETOIF) {
		if((NULL == (ia = ifatoia(ifa_ifwithdstaddr(sintosa(dst))))) ||
				(NULL == (ia = ifatoia(ifa_ifwithnet(sintosa(dst))))) ||
				(NULL == (ia = ifatoia(ifa_ifwithaddr(sintosa(dst)))))) {
			error = ENETUNREACH;
			goto failed;	
		} 
		ifp = ia->ia_ifp;
		ip->ip_ttl = 1;
	} else {
		if(NULL == ro->ro_rt) 
			rt_alloc(ro);
		if(NULL == ro->ro_rt) { 
			//没有匹配路由
			error = ENETUNREACH;
			goto failed;	
		}
		ia = ifatoia(ro->ro_rt->rt_ifa);
		ifp = ro->ro_rt->rt_ifp;
	}

	/* 如果没有在inpcb中指定IP地址（即没有绑定源地址），则将出口地址的IP填写到ip_src字段 */
	if(INADDR_ANY == ip->ip_src.s_addr)	
		ip->ip_src = ia->ia_addr.sin_addr;

	if(ip->ip_len <= ifp->if_mtu) {
		/* 转换字节序并计算校验和 */
		ip->ip_len = htons(ip->ip_len);
		ip->ip_off = htons(ip->ip_off);
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(m, hlen);
		if(error = (*ifp->if_output)(ifp, m, sintosa(dst), ro->ro_rt))
			goto failed;
	}

done:
	/* 释放ip_output函数分配的rtentry */
	if(ro == &iproute && !(flags & IP_ROUTETOIF) && ro->ro_rt)
		;//rt_free(ro->ro_rt);

	return error;

failed:
	mbuf_freem(m);
	goto done;
}

/**
 * 描述：插入选项数据到IP帧头，该函数在插入选项后，将会更新IP帧头中的hen和len字段
 * 参数：m：存放IP报文头的mbuf结构，mo：存放选项数据的mbuf结构
 */
static struct mbuf *ip_insertoptions(struct mbuf *m, struct mbuf *mo)
{
	int optlen;
	struct mbuf *n;
	struct ip *ip = mtod(mo, struct ip *);
	struct ip_option *opt = mtod(mo, struct ip_option *);

	if(ip->ip_len + optlen > IP_MAXPACKET)
		return m;

	optlen = mo->m_len - sizeof(opt->ipopt_dst);

	/* 长度不够或者第一个mbuf为EXT类型时需要新分配一个mbuf */
	if((m->m_flags & MF_EXT) || (m->m_data - optlen < m->m_pktdat)) {
		if(!(n = mbuf_getpkt(MT_DATA)))
			return m;
		//拷贝原来的PKT信息到新分配的mbuf中
		MBUF_COPY_PKTHDR(n, m);
		m->m_flags &= ~MF_PKTHDR;
		n->m_pkthdr.mp_len = m->m_pkthdr.mp_len + optlen;
		m->m_len -= sizeof(struct ip);
		m->m_data += sizeof(struct ip);
		n->m_next = m;
		m = n;
		m->m_len = sizeof(struct ip) + optlen;
	} else {
		m->m_data -= optlen;
		m->m_len += optlen;
		m->m_pkthdr.mp_len += optlen;
	}

	bcopy((caddr_t)ip, mtod(m, caddr_t), sizeof(struct ip));
	ip = mtod(m, struct ip *);
	bcopy((caddr_t)opt->ipopt_list, (caddr_t)(ip + 1), optlen);
	
	/* 更新IP头中的长度信息 */
	ip->ip_hlen = (sizeof(struct ip) + optlen) >> 2;
	ip->ip_len += optlen;

	if(opt->ipopt_dst.s_addr != INADDR_NONE)
		ip->ip_dst = opt->ipopt_dst;

	return m;
}
