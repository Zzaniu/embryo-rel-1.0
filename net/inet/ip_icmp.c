
/**
 * Copyright(c) 2018-3-5 Shangwen Wu	
 *
 * ICMP协议相关函数，
 * 
 */
#include <common.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/endian.h>					
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/list.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/errno.h>
#include <sys/protocol.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

static int icmpbmcastecho = 0;		//是否支持对广播地址的回显请求
static int icmpmaskrepl = 0;		//是否支持子网掩码查询请求

static struct sockaddr_in icmpdst = {sizeof(struct sockaddr_in), AF_INET};
static struct sockaddr_in icmpsrc = {sizeof(struct sockaddr_in), AF_INET};
static struct sockaddr_in icmpgw = {sizeof(struct sockaddr_in), AF_INET};

/**
 * 描述：icmp协议输入函数
 * 参数：m，保存整个基于icmp报文的ip包缓冲区（ip_len不包括IP头长度）
 * 		 hlen，ip头长度
 */
int icmp_input(struct mbuf *m, ...)
{
	struct ip *ip = mtod(m, struct ip *);
	struct icmp *icp;
	struct in_ifaddr *ia;
	ulong hlen, i, icmplen = ip->ip_len;
	va_list ap;
	int type, code;
	extern uchar ip_protox[];	//defined in ip_input.c
	extern struct protocol inet_protos[];	//defined in in_proto.c
	void *(*ctlfunc)(int, struct sockaddr *, void *);/* 下层协议的特殊处理 */
	
	ICMPLVL_DBG("In icmp_input\n");

	va_start(ap, m);
	hlen = va_arg(ap, ulong);
	va_end(ap);

	/* 检查icmp报文长度 */
	if(icmplen < ICMP_MINLEN)
		goto out_err;
	/* 将IP头、ICMP头以及部分数据前移到第一个mbuf节点 */
	i = hlen + min(icmplen, ICMP_ADVLENMIN);
	if(m->m_len < i && !(m = mbuf_pullup(m, i)))
		return -1;

	ip = mtod(m, struct ip *);

	/* 计算ICMP的校验和 */
	m->m_data += hlen;
	m->m_len -= hlen;
	icp = mtod(m, struct icmp *);
	if(in_cksum(m, icmplen))
		goto out_err;	//丢弃校验和出错的报文	
	m->m_data -= hlen;
	m->m_len += hlen;
	/* type非法时交给RAW帧类型的监听程序处理 */
	type = icp->icmp_type;
	code = icp->icmp_code;
	if(type > ICMP_MAXTYPE)	
		goto raw;	

	switch(type) {
		case ICMP_UNREACH:
			/* 调整code为对应的ctlinput函数对应的命令值 */
			switch(code) {	
				case ICMP_UNREACH_NET:
				case ICMP_UNREACH_HOST:
				case ICMP_UNREACH_PROTOCOL:
				case ICMP_UNREACH_PORT:
				case ICMP_UNREACH_SRCFAIL:
					code += PRC_UNREACH_NET;
					break;
				case ICMP_UNREACH_NEEDFRAG:
					code = PRC_MSGSIZE;
					break;	
				case ICMP_UNREACH_NET_UNKNOWN:
				case ICMP_UNREACH_NET_PROHIB:
				case ICMP_UNREACH_TOSNET:
					code = PRC_UNREACH_NET;
					break;
				case ICMP_UNREACH_HOST_UNKNOWN:
				case ICMP_UNREACH_ISOLATED:
				case ICMP_UNREACH_HOST_PROHIB:
				case ICMP_UNREACH_TOSHOST:
				case ICMP_UNREACH_HOST_PRECEDENCE:
				case ICMP_UNREACH_FILTER_PROHIB:
				case ICMP_UNREACH_PRECEDENCE_CUTOFF:
					code = PRC_UNREACH_HOST;
					break;
				default:
					goto badcode;	
			}
			goto deliver;

		case ICMP_TIMXCEED:
			if(code > 1)
				goto badcode;
			code += PRC_TIMXCEED_INTRANS; 
			goto deliver;

		case ICMP_PARAMPROB:
			if(code > 1)
				goto badcode;
			code = PRC_PARAMPROB; 
			goto deliver;

		case ICMP_SOURCEQUENCH:
			if(code > 1)
				goto badcode;
			code = PRC_QUENCH; 

deliver:		
			if(icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp) ||
					icp->icmp_ip.ip_hlen < (sizeof(struct ip) >> 2))
				goto out_err;	//丢弃ICMP长度出错的报文
			if(IN_MULTICAST(icp->icmp_ip.ip_dst.s_addr))		
				goto badcode;	//来自于广播包的差错报文将被认为错误包交给上层协议
			/* 在最终提交给上层协议之前需要对引起出错的报文进行对应协议的cltinput处理 */
			icp->icmp_ip.ip_len = ntohs(icp->icmp_ip.ip_len);
			icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
			ctlfunc = inet_protos[ip_protox[icp->icmp_ip.ip_pro]].pr_ctlinput;
			if(ctlfunc)
				(*ctlfunc)(code, sintosa(&icmpsrc), &icp->icmp_ip);
			break;

		case ICMP_ECHO:
			if(!icmpbmcastecho && (m->m_flags & (MF_BCAST | MF_MCAST)))
				break;//不支持广播多波地址的回显请求，交给上层协议处理
			icp->icmp_type = ICMP_ECHOREPLY;
			goto reflect;

		case ICMP_TSTAMP:
			if(!icmpbmcastecho && (m->m_flags & (MF_BCAST | MF_MCAST)))
				break;
			if(icmplen < ICMP_TSLEN)
				break;//长度出错
			/* 设置ICMP报文要求的接收和传送时间戳 */
			icp->icmp_type = ICMP_TSTAMPREPLY;
			icp->icmp_rtime = iptime();
			icp->icmp_ttime = icp->icmp_rtime; //这里发送时间=接收时间
			goto reflect;
		
		case ICMP_MASKREQ:
			if(!icmpmaskrepl)
				break;
			if(icmplen < ICMP_MASKLEN)
				break;//长度出错
			/* 返回本机接收IP地址（当接收IP包的目的地址不为单播地址时，使用IP源地址）所属的网络掩码 */
			if(INADDR_BROADCAST == ip->ip_dst.s_addr || INADDR_ANY == ip->ip_dst.s_addr)
				icmpdst.sin_addr = ip->ip_src;
			else
				icmpdst.sin_addr = ip->ip_dst;
			ia = ifatoia(ifaof_ifpforaddr(sintosa(&icmpdst), m->m_pkthdr.mp_recvif));	
			if(NULL == ia)
				break;//找不到掩码，将返回上层处理
			icp->icmp_type = ICMP_MASKREPLY;
			icp->icmp_mask = ia->ia_sockmask.sin_addr.s_addr;
			if(!ip->ip_src.s_addr) {
				if(ia->ia_ifp->if_flags & IFF_BROADCAST)
					ip->ip_src = ia->ia_broadaddr.sin_addr;
				else if(ia->ia_ifp->if_flags & IFF_POINT2POINT)
					ip->ip_src = ia->ia_dstaddr.sin_addr;
			}
	
reflect:
			/* 响应ICMP报文 */
			ip->ip_len += hlen;	/* ipv4_input函数中将ip_len减去了IP头长 */
			icmp_reflect(m);/* 由下层释放m */
			return 0;

		case ICMP_REDIRECT:
			/* 处理路由重定向 */
			if(code > 3)
				goto badcode;
			if(icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp) ||
					icp->icmp_ip.ip_hlen < (sizeof(struct ip) >> 2))
				goto out_err;	//丢弃ICMP长度出错的报文
			icmpgw.sin_addr = ip->ip_src;	
			icmpdst.sin_addr = icp->icmp_gwaddr;
			icmpsrc.sin_addr = ip->ip_dst;
			/* 传递给上层之前调整路由 */		
			rtredirect(sintosa(&icmpsrc), sintosa(&icmpdst), NULL, 
					RTF_GATEWAY|RTF_HOST, sintosa(&icmpgw), NULL);
			pfctlinput(PRC_REDIRECT_HOST, sintosa(&icmpsrc));
			break;

		/* 对于响应类型的ICMP报文，当前层不做处理，由上层RAW帧类型的监听程序处理 */
		case ICMP_ECHOREPLY:
		case ICMP_ROUTERADVERT:
		case ICMP_ROUTERSOLICIT:
		case ICMP_TSTAMPREPLY:
		case ICMP_IREQREPLY:
		case ICMP_MASKREPLY:
		default:
			break;
	}

badcode:
	/* 出现某些错误的字段时，该报文仍被传递给上层处理 */
raw:
	
	return rip_input(m, 0);

out_err:
	log(LOG_ERR, "Icmp_input fail\n");

	if(m)
		mbuf_freem(m);

	return -1;
}

/**
 * 描述：根据传入的引起出错的IP报文，组装并发送对应的ICMP差错报文
 * 注意：传入的m中IP头的ip_len、ip_off、ip_id字段为主机字节序，计算校验和时需要进行转换，
 * 		 并且ip_len为去掉IP头之后的长度
 */
void icmp_error(struct mbuf *n, int type, int code, ulong dest, struct ifnet *destifp)
{
	struct ip *oip = mtod(n, struct ip *), *nip;
	ulong icmplen, ohlen = oip->ip_hlen << 2;
	struct mbuf *m = NULL, m0;
	struct icmp *icp;

	/* 当ip报文进行了分片，并且不是第一片时，不发送icmp差错报文 */
	if(oip->ip_off & IP_OFFMASK)
		goto release;
	/* 当接收到的报文本身就为ICMP差错报文时，不再发送ICMP差错报文 */
	if(IPPROTO_ICMP == oip->ip_pro && type != ICMP_REDIRECT &&
			n->m_len >= ohlen + ICMP_MINLEN && 
			ICMP_INFOTYPE(((struct icmp *)((caddr_t)oip + ohlen))->icmp_type))
		goto release;
	/* 为广播或多播包时，不进行ICMP报文发送 */	
	if(n->m_flags & (MF_BCAST | MF_MCAST))
		goto release;
	
	if(!(m = mbuf_getpkt(MT_HEADER)))
		goto release;
	/* ICMP差错报文的数据部分包括引起出错的ip头+ip报文的前8个字节数据 */
	icmplen = ohlen + min(8, oip->ip_len);
	m->m_len = icmplen + ICMP_MINLEN;
	MH_ALIGN(m, m->m_len);			/* 数据指针往后移，以便向前插入icmp数据报文的ip头 */
	icp = mtod(m, struct icmp *);
	if(type > ICMP_MAXTYPE)
		goto release;
	icp->icmp_type = type;
	/* 某些需要进行设置的特殊字段 */
	if(ICMP_REDIRECT == type) {
		icp->icmp_gwaddr.s_addr = dest;	
	} else {
		icp->icmp_void = 0;
		if(ICMP_PARAMPROB == type) {
			icp->icmp_pptr = code;
			code = 0;
		} else if(ICMP_UNREACH == type && ICMP_UNREACH_NEEDFRAG == code && destifp) {
			icp->icmp_nextpmtu = htons(destifp->if_mtu);
		}
	}
	icp->icmp_code = code;
	/* 将引起出错的ip头（包括8字节数据）拷贝到icmp的数据部分 */
	bcopy((caddr_t)oip, (caddr_t)(nip = &icp->icmp_ip), icmplen);
	/* 调整ip_len、ip_off、ip_id的字节序，以便计算校验和 */
	nip->ip_id = htons(nip->ip_id);
	nip->ip_off = htons(nip->ip_off);
	nip->ip_len = htons(nip->ip_len);
	/* 计算ICMP数据中的IP头校验和 */
	m0.m_len = nip->ip_hlen << 2;
	m0.m_next = NULL;
	m0.m_data = (caddr_t)nip;
	nip->ip_sum = 0;
	nip->ip_sum = in_cksum(&m0, nip->ip_hlen << 2);

	/* 拷贝原IP头（不包括选项部分）作为ICMP差错报文的ip头 */
	if(m->m_data - sizeof(struct ip) < m->m_pktdat)
		goto release;		/* 空间不够 */
	m->m_len += sizeof(struct ip);
	m->m_data -= sizeof(struct ip);//向前插入IP头
	m->m_pkthdr.mp_len = m->m_len;
	m->m_pkthdr.mp_recvif = n->m_pkthdr.mp_recvif;
	nip = mtod(m, struct ip *);
	bcopy((caddr_t)oip, (caddr_t)nip, sizeof(struct ip));
	nip->ip_len = m->m_len; 
	nip->ip_hlen = sizeof(struct ip) >> 2;
	nip->ip_pro = IPPROTO_ICMP;
	nip->ip_tos = 0;
	icmp_reflect(m);
	if(n)
		mbuf_freem(n);

	return;

release:
	if(m)
		mbuf_freem(m);
	if(n)
		mbuf_freem(n);
}

/* 获取当前时间 */
n_time iptime(void)
{
	return 0;
}

/**
 * 描述：根据接收到的ip报文数据，向源地址返回一个响应数据包
 * 参数：m，保存接收到的ip报文数据
 * 注意：该函数将释放m所在空间
 */
void icmp_reflect(struct mbuf *m)
{
	struct ip *ip = mtod(m, struct ip *);
	struct in_addr tmp;
	struct in_ifaddr *ia;
	struct mbuf *opts = NULL;
	int optlen, opt, len, cnt;//bad code
	caddr_t cp;	

	/* 过滤不允许转发的广播地址以及A类特殊地址，但是回环地址除外 */
	if(!in_canforward(ip->ip_src) && 
			(IN_CLASSA_NET(ip->ip_src.s_addr) != IN_LOOPBACKNET)) {
		mbuf_freem(m);
		return ;
	}

	/** 
 	 * 设置响应IP数据包的目的地址为接收IP包的源地址，注意：当接收IP数据报文使用了
 	 * 源站路由选项时，ip_src表示的是最开始的源主机地址，而不一定为下一跳主机地址
 	 * ，下一跳的地址将在后面的代码中通过ip_srcroute函数保存在opt选项数据的开始位
 	 * 置，最后ip_output函数发送响应数据报文时又将通过ip_insertoptions函数将ip报文
 	 * 中的ip_src替换为下一跳地址（即选项的开始位置）
 	 */
	tmp = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	
	/* 查找接收IP的目的地址在本机的最优匹配，并将找到的最优匹配地址设置为响应报文的源地址 */
	list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
		/* 找到完全匹配的地址或者某个接口的广播地址 */
		if(tmp.s_addr == ia->ia_addr.sin_addr.s_addr)
			break;
		if((ia->ia_ifp->if_flags & IFF_BROADCAST) && 
				(tmp.s_addr == ia->ia_broadaddr.sin_addr.s_addr))
			break;
	}
	icmpdst.sin_addr = tmp;
	if(&ia->ia_list == &in_ifaddrlist) 			//未找到合适的ifaddr
		ia = ifatoia(ifaof_ifpforaddr(sintosa(&icmpdst), m->m_pkthdr.mp_recvif));	
	/* 没有找到匹配于入口地址的本机地址 */	
	if(NULL == ia) {
		mbuf_freem(m);
		return ;
	}
	ip->ip_src = ia->ia_addr.sin_addr;
	ip->ip_ttl = IPMAXTTL;

	optlen = (ip->ip_hlen << 2) -sizeof(struct ip);
	/* 包含选项数据时，需要将选项部分从m分离到opts中 */
	if(optlen > 0) {
		/* 对于源站路由选项数据从ip_srcroute返回，否则新建一个mbuf，并将选项拷贝到opts中 */
		if(!(opts = ip_srcroute()) && 
			(opts = mbuf_getpkt(MT_HEADER))) {
			/* 参加ip_options结构定义 */
			opts->m_len = sizeof(struct in_addr);
			mtod(opts, struct in_addr *)->s_addr = 0;	//对于非源站路由，无需设置选项的dst字段
		}
		/* opt分配成功 */
		if(opts) {
			cp = (caddr_t)(ip + 1);
			/* 将非源站选路的选项数据从ip报文中拷贝到opts */	
			for(cnt = optlen; cnt > 0; cnt -= len, cp += len) {
				opt = cp[IPOPT_CODE];
				if(IPOPT_EOL == opt) 
					break;
				else if(IPOPT_NOP == opt) 
					len = 1;
				else {
					len = cp[IPOPT_LEN];
					if(len <= 0 || len > cnt)
						break;
				}
				if(IPOPT_RR == opt || IPOPT_TS == opt) {
					bcopy(cp, mtod(opts, caddr_t) + opts->m_len, len);
					opts->m_len += len;
				}
			}
			/* 将opts长度修改4字节对齐 */
			if((cnt = opts->m_len % 4)) {
				for(; cnt; --cnt) {
					*(mtod(opts, caddr_t) + opts->m_len) = IPOPT_NOP;
					++opts->m_len;
				}
			}
		}
		/* 去掉ip数据包中的IP选项部分 */
		ip->ip_hlen = sizeof(struct ip) >> 2;
		ip->ip_len -= optlen;
		m->m_len -= optlen;
		if(m->m_flags & MF_PKTHDR)
			m->m_pkthdr.mp_len -= optlen;
		bcopy(mtod(m, caddr_t) + sizeof(struct ip) + optlen,
				 (caddr_t)(ip + 1), m->m_len - sizeof(struct ip));
	}
	/* 取出数据包中的多播和广播标识 */
	m->m_flags &= ~(MF_BCAST | MF_MCAST);
	/* 计算数据包校验和并发送该数据报文 */	
	icmp_send(m, opts);
	if(opts)
		mbuf_freem(opts);
}	

/**
 * 描述：发送ICMP报文，该函数将计算ICMP协议校验和
 */
void icmp_send(struct mbuf *m, struct mbuf *opts)
{
	struct ip *ip;
	struct icmp *icp;
	ulong hlen;

	ip = mtod(m, struct ip *);
	hlen = ip->ip_hlen << 2;
	/* 调整mbuf指向icmp数据报文，以便计算校验和 */
	m->m_data += hlen;
	m->m_len -= hlen;
	icp = mtod(m, struct icmp *);
	icp->icmp_cksum = 0;	//该字段也许要算入校验和，因此需要清空该字段
	/* 注意，ip_len必须进行了字节序转换 */
	icp->icmp_cksum = in_cksum(m, ip->ip_len - hlen);
	m->m_data -= hlen;
	m->m_len += hlen;
	/* 传递给下一层协议进行发送 */
	ip_output(m, opts, NULL, 0);
}


