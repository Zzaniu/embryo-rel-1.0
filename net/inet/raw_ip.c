
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * 原始IP协议相关函数，该模块用于处理类型为SOCK_RAW的socket网络通信，
 * 该类型将IP报头的组装交给用户自己实现
 * 
 */
#include <common.h>
#include <stdarg.h>
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
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_pcb.h>

static struct inpcbtable rawcptable; 
static ulong rip_sndspace = 8192;	//socket send buf size
static ulong rip_rcvspace = 8192;	//socket recv buf size
static struct sockaddr_in ripsrc = {sizeof(struct sockaddr_in), AF_INET};

/* 初始化inpcb链表 */
void rip_init(void)
{
	static int initialized = 0;

	if(!initialized) 
		init_inpcb(&rawcptable, 1);
}

/**
 * 描述：原始数据帧输入函数
 * 参数：m，数据缓冲区
 * 注意：该函数最终将释放m所占用的空间
 */
int rip_input(struct mbuf *m, ...)
{
	struct in_pcb *inp;
	struct ip *ip = mtod(m, struct ip *);
	struct socket *last = NULL;
	struct mbuf *n;	

	ripsrc.sin_addr = ip->ip_src;
	
	list_for_each_entry(inp, &rawcptable.int_pcbqueue, inp_queue) {
		/* 查找匹配的socket，并将数据添加到socket的接收队列 */
		if(inp->inp_ip.ip_pro && 						//协议匹配
				inp->inp_ip.ip_pro != ip->ip_pro)
			continue;
		if(inp->inp_faddr.s_addr != INADDR_ANY &&		//远端绑定地址匹配
				inp->inp_faddr.s_addr != ip->ip_src.s_addr)
			continue;
		if(inp->inp_laddr.s_addr != INADDR_ANY &&		//本地绑定地址匹配
				inp->inp_laddr.s_addr != ip->ip_dst.s_addr)
			continue;
		if(last) {
			if((n = mbuf_copy(m, 0, M_COPYALL)) != NULL) {
				if(!sbappendaddr(&last->soc_rcvsb, sintosa(&ripsrc), n, NULL))
					sowakeup(last, &last->soc_rcvsb);
				else //丢弃数据报文
					mbuf_freem(n);
			}		
		}
		last = inp->inp_sock;
	}
	if(last) {
		if(!sbappendaddr(&last->soc_rcvsb, sintosa(&ripsrc), m, NULL))
			sowakeup(last, &last->soc_rcvsb);
		else //丢弃数据报文
			mbuf_freem(m);
	} else {
		if(ip->ip_pro != IPPROTO_ICMP)
			icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PROTOCOL, 0, NULL);
		else
			mbuf_freem(m);
	}

	return 0;
}

/* 
 * 描述：原始IP的输出处理，该函数将组装一个IP头（或者调用者自己组装好） 
 * 		 然后将该数据报文交给ip_output进行下一曾处理，该函数将释放m链
 * 		 所占用的内存空间，无论成功与否。
 * 返回：错误返回errno
 */
int rip_output(struct mbuf *m, ...)
{
	struct socket *so;
	in_addr_t dst;
	struct in_pcb *inp;
	va_list ap;
	struct ip *ip;
	int flags;

	va_start(ap, m);
	so = va_arg(ap, struct socket *);
	dst = va_arg(ap, in_addr_t);
	va_end(ap);

	flags = (so->soc_options & SO_DONTROUTE ? IP_ROUTETOIF : 0) | IP_ALLOWBROADCAST;
	inp = socktoinpcb(so);

	if(!(inp->inp_flags & INP_HDRINCL)) {
		if(m->m_pkthdr.mp_len + sizeof(struct ip) > IP_MAXPACKET) {
			mbuf_freem(m);
			return EMSGSIZE;
		}

		if(!(m = mbuf_reserve(m, sizeof(struct ip)))) 
			return ENOBUFS;
		
		/* 组装一个默认的IP协议头 */
		ip = mtod(m, struct ip *);
		ip->ip_tos = 0;
		ip->ip_len = m->m_pkthdr.mp_len;
		ip->ip_hlen = sizeof(struct ip) >> 2;//ip_output函数中将会添加选项长度
		ip->ip_off = 0;
		ip->ip_pro = inp->inp_ip.ip_pro;
		ip->ip_src = inp->inp_laddr;//如果此处没有填写合法IP，那么ip_output将填写发送接口的IP地址
		ip->ip_dst.s_addr = dst;
		ip->ip_ttl = IPMAXTTL;
	} else {
		if(m->m_pkthdr.mp_len > IP_MAXPACKET) {
			mbuf_freem(m);
			return EMSGSIZE;
		}
		/* 修正部分上层组装的IP字段 */
		ip = mtod(m, struct ip *);
		ip->ip_len = ntohs(ip->ip_len);
		ip->ip_off = ntohs(ip->ip_off);
		if(ip->ip_hlen < (sizeof(struct ip) >> 2) || \
				((ip->ip_hlen > (sizeof(struct ip) >> 2)) && inp->inp_option) || \
				ip->ip_len < (ip->ip_hlen >> 2) || 
				ip->ip_len > m->m_len) {
			mbuf_freem(m);
			return EINVAL;
		}
		flags |= IP_RAWOUTPUT;	
	}

	return ip_output(m, inp->inp_option, NULL, flags);
}

/* 原始IP协议命令处理函数，注意：该函数将会释放m变量的空间 */
int rip_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *n, struct mbuf *control)
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
			panic("rip_usrreq inpcb not null");	
		if(!(so->soc_stat & SS_PRIV)) {
			ret = EACCES;
			break; 
		}
			
		if(ret = soreserve(so, rip_sndspace, rip_rcvspace))
			break;
		if(ret = alloc_inpcb(so, &rawcptable)) { 
			sbrelease(&so->soc_sndsb);
			sbrelease(&so->soc_rcvsb);
			break;
		}
		inp = socktoinpcb(so);
		inp->inp_ip.ip_pro = (long)n;
		break;
	case PRU_SEND:
		{
			in_addr_t dst;
			if(so->soc_stat & SS_ISCONNECTED) {
				if(n != NULL) {
					ret = EISCONN;		
					break;
				}
			} else {
				if(NULL == n) {
					ret = ENOTCONN;
					break;
				}
				dst = mtod(n, struct sockaddr_in *)->sin_addr.s_addr;
			}
			ret = rip_output(m, so, dst);
			m = NULL;				//rip_output无论操作是否成功都将释放m的内存空间
			break;
		}
	case PRU_DETACH:
		if(!inp)
			panic("rip_usrreq inpcb is null");	
		free_inpcb(so);
		break;
	case PRU_RCVOOB:
	case PRU_RCVD:
		ret = EOPNOTSUPP;
		break;
	default:
		panic("rip_usrreq unkown req");	
	}

out:
	if(m)
		mbuf_freem(m);

	return ret;
}

