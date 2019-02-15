
/**
 * Copyright(c) 2017-8-2 Shangwen Wu	
 *
 * 网络回环接口
 * 
 */

#include <common.h>
#include <sys/types.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/netisr.h>
#include <mach/intr.h>

#include <net/if.h>
#include <net/if_type.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip_var.h>
#include <netinet/in_var.h>

#define LOOPMTU	32768

/* LOOPBACK调试开关 */
#define LOOPBACK_DEBUG		0
#if LOOPBACK_DEBUG
#define LOOPBACK_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define LOOPBACK_DBG(fmt, args...)		do{}while(0)	
#endif

static int looutput(struct ifnet *ifp, struct mbuf *m, struct sockaddr *sa, struct rtentry *rt);
static int loioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data);

struct ifnet *lo0;

void loopattach(int n)
{
	int i;
	struct ifnet *ifp;

	for(i = 0; i < n; ++i) {
		if(NULL == (ifp = (struct ifnet *)kmem_zmalloc(sizeof(struct ifnet)))) {
			log(LOG_ERR, "loopattach: out of memory\n");
			return;
		}
		sprintf(ifp->if_xname, "loop%d", i);		
		ifp->if_flags = IFF_LOOPBACK;
		ifp->if_output = looutput;
		ifp->if_ioctl = loioctl;
		ifp->if_type = IFT_LOOPBACK;
		ifp->if_addrlen = 0;
		ifp->if_mtu = LOOPMTU;
		ifp->if_metric = 1;
		if(if_attach(ifp, 1)) {
			kmem_free(ifp);
			return;
		}
		if(0 == i)
			lo0 = ifp;
	}
}

static int looutput(struct ifnet *ifp, struct mbuf *m, struct sockaddr *sa, struct rtentry *rt)
{
	int s;
	struct ifqueue *ifq;

	LOOPBACK_DBG("loopback output a packet\n");

	if((ifp->if_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
		return ENETDOWN;

	if(!(m->m_flags & MF_PKTHDR))
		panic("loopout mbuf is not pkthdr");

	m->m_pkthdr.mp_recvif = ifp;		//接收接口指向自己

	ifp->if_obytes += m->m_pkthdr.mp_len;
	ifp->if_opackets++;

	switch(sa->sa_family) {
		case AF_INET:
			ifq = &ipintrq;
			break;
		default:
			log(LOG_ERR, "%s: address family %d no support\n", ifp->if_xname, sa->sa_family);
			return EAFNOSUPPORT;
	}
	s = splimp();
	/* 插入IP接收队列 */
	if(IFQ_ISFULL(ifq)) {
		log(LOG_ERR, "%s: recv queue is full\n", ifp->if_xname);
		IFQ_DROP(ifq);
		splx(s);
		return ENOBUFS;
	}
	
	IFQ_ENQUEUE(ifq, m);

	//触发网络中断
	schednetisr(NETISR_IP);

	ifp->if_ibytes += m->m_pkthdr.mp_len;
	ifp->if_ipackets++;

	splx(s);
	return 0;
}

/**
 * 描述：loop接口对路由相关额外处理
 */
static void lo_rtrequest(int cmd, struct rtentry *rt, struct sockaddr *sa)
{
	if(rt)
		rt->rt_rmx.rmx_mtu = LOOPMTU;
}

/**
 * 描述：底层接口的ioctl函数
 */
static int loioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
	struct ifreq *ifr;
	struct ifaddr *ifa;

	switch(cmd) {
		case SIOCSIFADDR:
			ifa = (struct ifaddr *)data;
			if(!(ifp->if_flags & IFF_UP))
				ifp->if_flags |= IFF_UP | IFF_RUNNING;
			if(ifa)	
				ifa->ifa_rtrequest = lo_rtrequest;
			break;
		case SIOCSIFFLAGS:	//bad code
			ifr = (struct ifreq *)data;
			if((ifr->ifr_flags & IFF_UP) && !(ifp->if_flags & IFF_UP))
				ifp->if_flags |= IFF_UP | IFF_RUNNING;
			if(!(ifr->ifr_flags & IFF_UP) && (ifp->if_flags & IFF_UP))
				ifp->if_flags &= ~(IFF_RUNNING | IFF_UP);
			break;
		defualt:
			return EOPNOTSUPP;
	}
	
	return 0;
}

