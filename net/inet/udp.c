
/**
 * Copyright(c) 2017-8-21 Shangwen Wu	
 *
 * UDP协议相关函数，
 * 
 */
#include <common.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/endian.h>					
#include <sys/system.h>
#include <sys/syslog.h>
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
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/in_pcb.h>

/* in_pcb哈希表大小 */
#define UDPHASHSIZE	128

/* UDP协议是否计算校验和 */
static int udpchksum = 1;

/* UDP协议控制hash表 */
static struct inpcbtable udpcptable; 
static unsigned int udphashsize = UDPHASHSIZE;

static ulong udp_sndspace = (8192 + 1024);	//socket send buf size
static ulong udp_rcvspace = 40 * (1024 + sizeof(struct sockaddr_in));	//socket recv buf size

/**
 * 描述：UDP报文输出函数
 * 参数：m，包含UDP数据部分；in_pcb，协议控制结构；addr，包含地址信息；control，包含控制数据
 */
int udp_output(struct mbuf *m, ...)
{
	int err = 0;
	va_list ap;
	struct mbuf *addr, *control;
	struct in_pcb *inp;
	int flags, inpflags, s;
	struct udpip *ui;
	struct ip *ip;
	ulong len;
	struct in_addr laddr;

	UDPLVL_DBG("in udp_output\n");

	va_start(ap, m);
	inp = va_arg(ap, struct in_pcb *);
	addr = va_arg(ap, struct mbuf *);
	control = va_arg(ap, struct mbuf *);
	va_end(ap);

	/* UDP报文不使用control数据 */
	if(control)
		mbuf_freem(control);
	
	if(addr) {
 		if(inp->inp_faddr.s_addr != INADDR_ANY) {
			err = EISCONN;
			goto out;
		}
		/** 
 		 * 本地地址和标识位可能会被inpcb_connect函数修改，但是这里并不保存端口，
 		 * 也就是说在socket关闭前，都只会使用第一次分配的端口号;
 		 */
		inpflags = inp->inp_flags;
		laddr = inp->inp_laddr;
		s = splsoftnet();
		if(err = inpcb_connect(inp, addr)) {
			splx(s);
			goto out;
		}
	} else {
 		if(INADDR_ANY == inp->inp_faddr.s_addr) {
			err = ENOTCONN;
			goto out;
		}
	}

	/* 计算伪头以及UDP报文校验和 */
	len = m->m_pkthdr.mp_len;
	if(len + sizeof(struct udpip) > IP_MAXPACKET) {
		err = EMSGSIZE;
		mbuf_freem(m);
		goto unconnect;
	}

	m = mbuf_reserve(m, sizeof(struct udpip));
	if(!m) {
		err = ENOBUFS;
		//mbuf_reserve将释放m空间
		goto unconnect;
	}
	ui = mtod(m, struct udpip *);
	bzero(ui->ui_resv, sizeof(ui->ui_resv));
	ui->ui_pro = IPPROTO_UDP; 
	ui->ui_len = htons(sizeof(struct udp) + len);	/* 长度为UDP报文头+数据部分 */
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;
	ui->ui_cksum = 0;
	if(udpchksum) {
		/* 注意：实际上校验和长度为伪头+UDP报文长度，但是因为udpip中的多余数据为0，所以这里并不影响校验和 */
		ui->ui_cksum = in_cksum(m, sizeof(struct udpip) + len);
		/* 校验和为0表示UDP发送方没有计算校验和 */
		if(0 == ui->ui_cksum)
			ui->ui_cksum = 0xffff;
	}

	/* 组装IP头以及UDP报文头 */
	ip = (struct ip *)ui;
	//ip->ip_sum = 0;	//ip_output将计算IP头校验和
	ip->ip_hlen = sizeof(struct ip) >> 2;
	ip->ip_tos = inp->inp_ip.ip_tos;
	ip->ip_ttl = inp->inp_ip.ip_ttl;
	ip->ip_len = sizeof(struct udpip) + len;//ip_output进行网络字节序转换

	/* 通过IP层发送该报文 */
	flags = inp->inp_sock->soc_options & (SO_DONTROUTE | SO_BROADCAST);
	err = ip_output(m, inp->inp_option, &inp->inp_ro, flags);

unconnect:
	if(addr) {
		/* 这里建立的只是一种临时绑定关系，如果addr改变，需要重新建立绑定关系 */
		inpcb_disconnect(inp);
		splx(s);
		inp->inp_flags = inpflags;
		inp->inp_laddr = laddr;
	}

	return err;

out:
	mbuf_freem(m);

	return err;
}

/**
 * 描述：UDP输入函数
 * 参数：m，数据报文，包括IP头；hlen，IP协议首部长度，包括选项数据长度
 */
int udp_input(struct mbuf *m, ...)
{
	va_list ap;
	ulong hlen, ohlen, ulen;
	struct ip *ip, save_ip;
	struct udpip *uip;
	struct udp *udp;
	struct sockaddr sa = {0};
	struct sockaddr_in *src;
	struct socket *last;
	struct mbuf *n, *control = NULL;
	struct in_pcb *inp;
	
	UDPLVL_DBG("in udp_input\n");

	va_start(ap, m);
	hlen = va_arg(ap, ulong);
	va_end(ap);
	ohlen = hlen;

	ip = mtod(m, struct ip *);
	switch (ip->ip_ver) {
		case 4:
			sa.sa_family = AF_INET;	
			break;
		default:
			log(LOG_WARNING, "Unkown IP version\n");
			goto discard;
	}

	/* 去掉IP选项数据 */
	if(hlen > sizeof(struct ip)) {
		ip_stripoptions(m, NULL);
		hlen = sizeof(struct ip);
	}

	/* 提取IP & UDP首部 */
	if(m->m_len < sizeof(struct udpip))	{
		if(!(m = mbuf_pullup_big(m, sizeof(struct udpip)))) {
			log(LOG_ERR, "mbuf_pullup_big error\n");
			return -1;
		}
		ip = mtod(m, struct ip *);
	}
	save_ip = *ip;
	uip = mtod(m, struct udpip *);
	udp = &uip->ui_udp;
	
	/* 检查UDP报文长度 */
	ulen = ntohs(udp->udp_ulen);
	if(ulen != m->m_pkthdr.mp_len - hlen) {
		if(ulen < sizeof(struct udp) || 
			ulen > m->m_pkthdr.mp_len - hlen) {
			log(LOG_ERR, "UDP length bad\n");
			goto discard;
		} else 
			mbuf_trim(m, m->m_pkthdr.mp_len - hlen - udp->udp_ulen, 1);
	}

	/* 检验UDP校验和，如果发送方计算了校验和 */
	if(udp->udp_cksum) {
		/* 组装伪头部 */
		bzero(uip->ui_resv, sizeof(uip->ui_resv));
		uip->ui_len = udp->udp_ulen;
		if(in_cksum(m, sizeof(struct ip) + ulen)) {
			log(LOG_ERR, "UDP checksum error\n");
			goto discard;
		} 
	}

	switch(sa.sa_family) {
		case AF_INET:
			src = satosin(&sa);
			src->sin_addr = ip->ip_src;
			src->sin_len = sizeof(struct sockaddr_in);
			src->sin_port = udp->udp_sport;
			break;
	}

	/** 
 	 * 如果报文目的IP地址为多播或者广播地址，则对每一个使用当前报文
 	 * 目的端口和地址的socket任务发送一个该报文的拷贝
 	 *
 	 */
	if(IN_MULTICAST(ip->ip_dst.s_addr) ||
			in_broadcast(ip->ip_dst)) {
		last = NULL;
	
		/* 去掉IP & UDP报文首部 */
		hlen += sizeof(struct udp);
		m->m_data += hlen;
		m->m_len -= hlen;
		m->m_pkthdr.mp_len -= hlen;

		list_for_each_entry(inp, &udpcptable.int_pcbqueue, inp_queue) {
			/* 查找匹配的socket，并将数据添加到socket的接收队列 */
			if(inp->inp_lport != udp->udp_dport)			//本地绑定端口匹配
				continue;
			if(inp->inp_laddr.s_addr != INADDR_ANY &&		//本地绑定地址匹配
					inp->inp_laddr.s_addr != ip->ip_dst.s_addr)
				continue;
			if(inp->inp_faddr.s_addr != INADDR_ANY &&		//远端绑定地址和端口匹配
					(inp->inp_faddr.s_addr != ip->ip_src.s_addr || 
					inp->inp_fport != udp->udp_sport))
				continue;
			/* 找到匹配的socket */
			if(last) {
				if((n = mbuf_copy(m, 0, M_COPYALL)) != NULL) {
					if(!sbappendaddr(&last->soc_rcvsb, sintosa(src), n, NULL))
						sowakeup(last, &last->soc_rcvsb);
					else //丢弃数据报文
						mbuf_freem(n);
				}		
			}
			last = inp->inp_sock;
			/* 如果发现有socket未设置端口和地址重用，那么将退出向其他socket进行多播发送 */
			if(!last->soc_options & (SO_REUSEADDR | SO_REUSEPORT))
				break;
		}
		/* 对于任何接收到的多播报文均不发送ICMP差错报文 */
		if(last) {
			if(!sbappendaddr(&last->soc_rcvsb, sintosa(src), m, NULL))
				sowakeup(last, &last->soc_rcvsb);
			else //丢弃数据报文
				goto discard;	
		} else 
			goto discard;	
		
		return 0;
	}

	/* 对于单播报文，采用hash查找inpcb */
	if(!(inp = inpcb_hashlookup(&udpcptable, 
			ip->ip_src, udp->udp_sport,
			ip->ip_dst, udp->udp_dport))) {
		/* 进行通配查找 */
		if(!(inp = inpcb_lookup(&udpcptable, INPCBLOOKUP_WILDCARD, ip->ip_src, udp->udp_sport,
				ip->ip_dst, udp->udp_dport))) {
			/* 找不到任何匹配的socket */
			if(m->m_flags & (MF_BCAST | MF_MCAST))
				goto discard;
			/* 单播报文，将返回端口不可达差错报文 */
			*ip = save_ip;
			ip->ip_len += ohlen;//ipv4_input函数将ip_len修改为数据长度，而不包括IP头长
			icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PORT, 0, NULL);
			m = NULL;		//icmp_error负责对m进行释放
			goto discard;
		}
	}
		
	/* 暂不支持向应用层返回控制数据 */
	if(inp->inp_flags & INP_CONTROLOPS) {
		log(LOG_ERR, "INP_CONTROLOPS unspported!\n");
		goto discard;
	}

	/* 去掉IP & UDP报文首部 */
	hlen += sizeof(struct udp);
	m->m_data += hlen;
	m->m_len -= hlen;
	m->m_pkthdr.mp_len -= hlen;

	/* 将数据插入到socket的接收队列 */
	if(!sbappendaddr(&inp->inp_sock->soc_rcvsb, sintosa(src), m, control))
		sowakeup(inp->inp_sock, &inp->inp_sock->soc_rcvsb);
	else //丢弃数据报文
		goto discard;

	return 0;

discard:
	if(control)
		mbuf_freem(control);
	if(m)
		mbuf_freem(m);

	return -1;
}

/**
 * 描述： UDP协议命令处理函数 
 */
int udp_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *n, struct mbuf *control)
{
	int ret = 0;
	struct in_pcb *inp = socktoinpcb(so);

	if(PRU_CONTROL == req) 
		return in_control(so, (ulong)m, (void *)n, (struct ifnet *)control);

	if(req != PRU_ATTACH && NULL == inp) 
		return EINVAL;

	switch(req) {
	case PRU_ATTACH:
		if(inp != NULL)
			panic("udp_usrreq inpcb not null");	
		
		if(ret = soreserve(so, udp_sndspace, udp_rcvspace))
			break;
		if(ret = alloc_inpcb(so, &udpcptable)) { 
			sbrelease(&so->soc_sndsb);
			sbrelease(&so->soc_rcvsb);
			break;
		}
		inp = socktoinpcb(so);
		inp->inp_ip.ip_ttl = IPDEFTTL;
		break;
	case PRU_SEND:
		return udp_output(m, inp, n, control);
	case PRU_BIND:
		ret = inpcb_bind(inp, n);
		break;
	case PRU_DETACH:
		if(!inp)
			panic("udp_usrreq inpcb is null");	
		free_inpcb(so);
		break;
	default:
		panic("udp_usrreq unkown req");	
	}

release:
	if(control) {
		log(LOG_ERR, "udp_usrreq: UDP control data unexpectedly retained\n");
		mbuf_freem(control);
	}
	if(m)
		mbuf_freem(m);

	return ret;
}

/* 初始化inpcb链表 */
void udp_init(void)
{
	static int initialized = 0;

	if(!initialized) 
		init_inpcb(&udpcptable, udphashsize);
}

