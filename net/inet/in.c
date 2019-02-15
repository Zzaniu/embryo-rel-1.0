
/**
 * Copyright(c) 2017-8-24 Shangwen Wu	
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
#include <sys/sockio.h>
#include <sys/errno.h>
#include <mach/intr.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

static ulong in_interface = 0;		//外部的internet接口

struct list_head in_ifaddrlist = LIST_HEAD_INIT(in_ifaddrlist);	//全局internet地址链表

/**
 * 描述：协议曾的ioctl函数，主要用于网络接口地址的相关访问
 * 注意：该函数仅遍历internet相关的地址链表，而非当前接口的任何协议域的地址链表，并且使用sockaddr_in参与比较
 *
 */
int in_control(struct socket *so, ulong cmd, void *data, struct ifnet *ifp)
{
	struct in_ifaddr *ia = NULL;;
	struct ifreq *ifr = (struct ifreq *)data;
	struct in_ifaliasreq *iifar = (struct in_ifaliasreq *)data;

	if(NULL == ifp)
		return EINVAL;

	/* 预处理 */
	switch(cmd) {
		case SIOCAIFADDR:
		case SIOCDIFADDR:
			list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
				if(ia->ia_ifp == ifp && \
						satosin(&ifr->ifr_addr)->sin_addr.s_addr == ia->ia_addr.sin_addr.s_addr)
					break;
			}
			if(SIOCDIFADDR == cmd) {
				if(&ia->ia_list == &in_ifaddrlist)	//未找到匹配的ifaddr
					return EADDRNOTAVAIL;
				else
					break;
			} else
				goto new_ifaddr;
		case SIOCSIFADDR:
		case SIOCSIFDSTADDR:
		case SIOCSIFNETMASK:
			if(!(so->soc_stat & SS_PRIV))
				return EPERM;
			list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
				if(ia->ia_ifp == ifp) 
					break;
			}

new_ifaddr:
			if(&ia->ia_list == &in_ifaddrlist) {	//未找到合适的ifaddr
				if(NULL == (ia = (struct in_ifaddr *)kmem_zmalloc(sizeof(struct in_ifaddr))))
					return ENOBUFS;
				ia->ia_ifa.ifa_addr = sintosa(&ia->ia_addr);
				ia->ia_ifa.ifa_dstaddr = sintosa(&ia->ia_dstaddr);
				ia->ia_ifa.ifa_netmask = sintosa(&ia->ia_sockmask);
				ia->ia_ifp = ifp;
				ia->ia_sockmask.sin_len = 8;
				if(ifp->if_flags & IFF_BROADCAST) {
					ia->ia_broadaddr.sin_len = sizeof(struct sockaddr_in);
					ia->ia_broadaddr.sin_family = AF_INET;
				}
				INIT_LIST_HEAD(&ia->ia_multlist);
				list_add_tail(&ia->ia_ifa.ifa_list, &ifp->if_addrlist);//将ifaddr插入到ifnet链表
				list_add_tail(&ia->ia_list, &in_ifaddrlist);//将in_ifaddr插入到全局internet链表
				if(ia->ia_flags & IFF_LOOPBACK)
					++in_interface;
			}
			break;
		case SIOCSIFBRDADDR:
			if(!(so->soc_stat & SS_PRIV))
				return EPERM;
			break;
		case SIOCGIFADDR:
		case SIOCGIFDSTADDR:
		case SIOCGIFNETMASK:
		case SIOCGIFBRDADDR:
			list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
				if(ia->ia_ifp == ifp) {
					/* 如果在传入的ifreq中指定了IP地址，那么将只查找匹配该地址的in_ifaddr */
					if(INADDR_ANY == satosin(&ifr->ifr_addr)->sin_addr.s_addr)
						break;
					else if(satosin(&ifr->ifr_addr)->sin_addr.s_addr == ia->ia_addr.sin_addr.s_addr)
						break;
				}
			}
			if(&ia->ia_list == &in_ifaddrlist) //未找到匹配的ifaddr
				return EADDRNOTAVAIL;
			break;
	}

	switch(cmd) {
		case SIOCGIFADDR:
			return copyout((caddr_t)satosin(&ifr->ifr_addr), (caddr_t)&ia->ia_addr, sizeof(struct sockaddr_in));
		case SIOCGIFDSTADDR:
			if(!(ifp->if_flags & IFF_POINT2POINT))
				return EINVAL;
			return copyout((caddr_t)satosin(&ifr->ifr_dstaddr), (caddr_t)&ia->ia_dstaddr, sizeof(struct sockaddr_in));
		case SIOCGIFNETMASK:
			return copyout((caddr_t)satosin(&ifr->ifr_addr), (caddr_t)&ia->ia_sockmask, sizeof(struct sockaddr_in));
		case SIOCGIFBRDADDR:
			if(!(ifp->if_flags & IFF_BROADCAST))
				return EINVAL;
			return copyout((caddr_t)satosin(&ifr->ifr_broadaddr), (caddr_t)&ia->ia_broadaddr, sizeof(struct sockaddr_in));
		case SIOCSIFADDR:
			return  in_ifinit(ifp, ia, satosin(&ifr->ifr_addr), 1);
		default:
			if(ifp->if_ioctl)
				return EOPNOTSUPP;
			return (*ifp->if_ioctl)(ifp, cmd, data);
	}

	return 0;
}


/**
 * 描述：该函数将为网络接口设置一个IP地址
 * 参数：ifp，需要设置的接口；ia，需要修改设置的in_ifaddr地址节点，sin，新设置的地址；scrub，是否删除原来地址的路由
 * 返回：错误码
 */
int in_ifinit(struct ifnet *ifp, struct in_ifaddr *ia, struct sockaddr_in *sin, int scrub)
{
	ushort flags = RTF_UP;
	int err = 0, s;
	struct sockaddr_in oldaddr;
	in_addr_t new = sin->sin_addr.s_addr;
	
	if(!IN_CLASSA(new) && !IN_CLASSB(new) && !IN_CLASSC(new)) 
		return EINVAL;

	oldaddr = ia->ia_addr;
	ia->ia_addr = *sin;

	/* 调用ifnet的ioctl函数用于检查接口地址以及初始化部分变量，比如初始化ifaddr级别的rtrequest函数 */
	s = splimp();
	if(ifp->if_ioctl && (err = ifp->if_ioctl(ifp, SIOCSIFADDR, (caddr_t)ia))) {
		splx(s);
		ia->ia_addr = oldaddr;
		return err;
	}
	splx(s);

	/* 需要删除之前的地址路由表项 */
	if(scrub) {
		ia->ia_ifa.ifa_addr = sintosa(&oldaddr);
		in_ifscrub(ifp, ia);	
		ia->ia_ifa.ifa_addr = sintosa(&ia->ia_addr);
	}

	/* 注意：以下代码中对掩码以及网络号的赋值都是进行过主机字节序到网络字节序转换过的 */
	if(IN_CLASSA(new))
		ia->ia_netmask = IN_CLASSA_MASK;
	else if(IN_CLASSB(new))
		ia->ia_netmask = IN_CLASSB_MASK;
	else 
		ia->ia_netmask = IN_CLASSC_MASK;
	/* 当没有设置子网掩码时，将网络掩码赋值给子网掩码 */
	if(!ia->ia_subnetmask) {
		ia->ia_subnetmask = ia->ia_netmask;
		ia->ia_sockmask.sin_addr.s_addr = ia->ia_netmask;
	} else //注意：一旦ia_subnetmask不为空时，ia_sockmask也必须设置正确
		ia->ia_netmask &= ia->ia_subnetmask;
	//截取sockmask掩码中字节为0的长度
	in_socktrim(&ia->ia_sockmask);
	//设置metric到ifaddr结构中
	ia->ia_ifa.ifa_metric = ifp->if_metric;
	/* 设置网络号 */
	ia->ia_net = new & ia->ia_netmask;
	ia->ia_subnet = new & ia->ia_subnetmask;

	/* 设置不同接口特性相关字段 */
	if(ifp->if_flags & IFF_BROADCAST) {
		//设置广播地址
		ia->ia_broadaddr.sin_addr.s_addr = new | ~ia->ia_subnetmask;
		ia->ia_netbroadcast.s_addr = new | ~ia->ia_netmask;
	} else if(ifp->if_flags & IFF_LOOPBACK) {
		ia->ia_dstaddr = ia->ia_addr;
		flags |= RTF_HOST;
	} else if(ifp->if_flags & IFF_POINT2POINT) {
		if(ia->ia_dstaddr.sin_family != AF_INET)
			return EINVAL;
		flags |= RTF_HOST;
 	}

	/* 为新设置的地址添加一条路由 */
	if(!(err = rtinit(&ia->ia_ifa, RTM_ADD, flags)))
		ia->ia_flags |= IFA_ROUTED;

	return err;
}

/* 删除之前指定的网络路由 */
void in_ifscrub(struct ifnet *ifp, struct in_ifaddr *ia)
{
	if(!(ia->ia_flags & IFA_ROUTED))
		return;
	if(ifp->if_flags & (IFF_POINT2POINT | IFF_LOOPBACK))
		rtinit(&ia->ia_ifa, RTM_DELETE, RTF_HOST);
	else
		rtinit(&ia->ia_ifa, RTM_DELETE, 0);
	ia->ia_flags &= ~IFA_ROUTED;
}

/* 截取网络掩码中为0的字节长度 */
void in_socktrim(struct sockaddr_in *sin)
{
	caddr_t cplim = (caddr_t)(&sin->sin_addr);
	caddr_t cp = (caddr_t)(&sin->sin_addr + 1);

	sin->sin_len = 0;
	while(--cp >= cplim)
		if(*cp) {
			sin->sin_len = cp - (char *)sin + 1;
			break;
		}
}

/* 判断传入的地址是否支持转发 */
int in_canforward(struct in_addr ina)
{
	in_addr_t net;

	/* 广播地址和预留地址不允许转发 */
	if(IN_EXPERIMENTAL(ina.s_addr) || IN_MULTICAST(ina.s_addr))
		return 0;
	
	/* A类网络号为0以及回环地址不允许转发 */
	if(IN_CLASSA(ina.s_addr)) {
		net = IN_CLASSA_NET(ina.s_addr);
		if(0 == net || IN_LOOPBACKNET == net)	
			return 0;
	}

	return 1;
}

/**
 * 描述：判断目标地址是否为本地某个接口对应的广播地址
 * 返回：目标地址为广播地址，则返回1，否则返回0
 */
int in_broadcast(struct in_addr addr)
{
	struct in_ifaddr *ia;

	if(INADDR_ANY == addr.s_addr ||
			INADDR_BROADCAST == addr.s_addr)
		return 1;

	list_for_each_entry(ia, &in_ifaddrlist, ia_list) {
		/** 
 		 * 对于不为单个主机子网，如果其对应的接口支持广播，并且目标地址为该接口的广播地址
 		 * 则认为该地址为某个接口对应的子网广播地址，此外如果目标地址的host号全0，也将认为
 		 * 目标地址是一个面向子网的广播地址
 		 */
		if(((ia->ia_subnetmask != 0xffffffff) &&
				(((ia->ia_ifp->if_flags & IFF_BROADCAST) && 
				(addr.s_addr == ia->ia_broadaddr.sin_addr.s_addr)) || 
				addr.s_addr == ia->ia_subnet)) || 
				/* 兼容老版本以主机号全0作为面向网络的广播地址 */
				addr.s_addr == ia->ia_netbroadcast.s_addr || addr.s_addr == ia->ia_net) 
			return 1;
	}

	return 0;
}

