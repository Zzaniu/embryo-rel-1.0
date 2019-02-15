
/**
 * Copyright(c) 2017-7-26 Shangwen Wu	
 *
 * 网络接口相关定义
 * 
 */

#include <common.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/endian.h>	
#include <sys/system.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <sys/list.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/socketvar.h>
#include <sys/protocol.h>
#include <sys/errno.h>
#include <mach/intr.h>

#include <net/if.h>
#include <net/radix.h>
#include <net/if_dl.h>

static LIST_HEAD(ifnetlist);

static struct ifaddr **ifnet_addrs = NULL;						//保存所有接口数据链路曾接口的ifaddr结构数组
static uint16_t ifnet_index = 0;								//数据链路曾接口的最大索引值

static int if_attachsetup(struct ifnet *ifp); 

int if_attach(struct ifnet *ifp, int tail)
{
	int err;
	
	INIT_LIST_HEAD(&ifp->if_addrlist);
	
	if((err = if_attachsetup(ifp)) != 0)
		return err;		

	if(tail) 
		list_add_tail(&ifp->if_list, &ifnetlist);
	else
		list_add(&ifp->if_list, &ifnetlist);

	return 0;
}

static int if_attachsetup(struct ifnet *ifp)
{
	struct sockaddr_dl *sdl;
	ulong n, masklen, namelen, socklen;;
	struct ifaddr **q, *ifa;
	static int ifnetlimits = 8;						//初始化的ifnet_addrs数据组大小为ifnetlimits * 2

	/* 空间不够时，将重新分配一段比原来大一倍的空间 */
	if(!ifnet_addrs || ifnet_index >= ifnetlimits) {
		n = (ifnetlimits << 1) * sizeof(struct ifaddr *);
		if(NULL == (q = (struct ifaddr **)kmem_malloc(n))) {
			log(LOG_ERR, "if_attachsetup: out of memory\n");
			return ENOMEM;
		}
		if(ifnet_addrs) {
			bcopy((caddr_t)ifnet_addrs, (caddr_t)q, n >> 1);
			kmem_free(ifnet_addrs);	
		}
		ifnet_addrs = q;
		ifnetlimits <<= 1;
	}

	namelen = strlen(ifp->if_xname);
	masklen = offsetof(struct sockaddr_dl, sdl_data) + namelen;
	socklen = masklen + ifp->if_addrlen;
	if(socklen < sizeof(struct sockaddr_dl))
		socklen = sizeof(struct sockaddr_dl);
	socklen = ROUNDUP(socklen);

	/* 为每个ifnet分配一个大小为sizeof(ifaddr) + 2*sizeof(sockaddr_dl)的空间 */
	if(NULL == (ifa = (struct ifaddr *)kmem_zmalloc(sizeof(struct ifaddr) + 2 * socklen))) {
		log(LOG_ERR, "if_attachsetup: out of memory\n");
		return ENOMEM;
	}
	ifnet_addrs[ifnet_index] = ifa;

	ifp->if_index = ifnet_index++;
	list_add(&ifa->ifa_list, &ifp->if_addrlist);
	sdl = (struct sockaddr_dl *)(ifa + 1);
	sdl->sdl_len = socklen;
	sdl->sdl_nlen = namelen;
	sdl->sdl_family = AF_LINK;
	sdl->sdl_index = ifp->if_index;
	/* 下面两个字段将在具体接口的attach函数中别重写 */
	sdl->sdl_type = ifp->if_type;
	sdl->sdl_alen = ifp->if_addrlen;
	bcopy((caddr_t)ifp->if_xname, (caddr_t)sdl->sdl_data, namelen);
	ifa->ifa_addr = (struct sockaddr *)sdl;
	sdl = (struct sockaddr_dl *)((caddr_t)sdl + socklen);
	sdl->sdl_len = masklen;
	while(namelen)
		sdl->sdl_data[--namelen] = 0xff;
	ifa->ifa_netmask = (struct sockaddr *)sdl;
	ifa->ifa_ifp = ifp;

	return 0;
} 

static inline equal(struct sockaddr *s1, struct sockaddr *s2)
{
	return (0 == bcmp((caddr_t)s1, (caddr_t)s2, s1->sa_len));
}

/* 
 * 查询当前接口下是否存在与目标地址位于同一网络域，且地址与目标地址位于同一网络
 * （或者匹配于回环接口或者P2P接口）的ifaddr 
 * 返回：当没有找到匹配的接口地址时，将返回NULL
 */
struct ifaddr *ifaof_ifpforaddr(struct sockaddr *addr, struct ifnet *ifp)
{
	struct ifaddr *ifa;
	sa_family_t af = addr->sa_family;
	caddr_t cp, cp1, cp2, cplim;

	if(af > AF_MAX)
		return NULL;
	list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) {
		if( ifa->ifa_addr->sa_family != af) 
			continue;
		if(!ifa->ifa_netmask) {
			if(equal(addr, ifa->ifa_addr) || \
					(ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return ifa;
			continue;
		}
		cp = addr->sa_data;	
		cp1 = ifa->ifa_addr->sa_data;	
		cp2 = ifa->ifa_netmask->sa_data;	
		cplim = (caddr_t)ifa->ifa_netmask + ifa->ifa_netmask->sa_len;
		while(cp2 < cplim) 
			if((*cp++ ^ *cp1++) & *cp2++)
				break;
		if(cp2 == cplim)
			return ifa;
	}

	return NULL;
}

/* 查找是否有对端地址匹配的点对点类型接口 */
struct ifaddr *ifa_ifwithdstaddr(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;

	list_for_each_entry(ifp, &ifnetlist, if_list) {
		if(ifp->if_flags & IFF_POINT2POINT) {
			list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) {
				if(ifa->ifa_addr->sa_family != addr->sa_family || \
						!ifa->ifa_dstaddr)
					continue;
				if(equal(addr, ifa->ifa_dstaddr))
					return ifa;
			}
		}
	}
	return NULL;
}

/* 查找是否存在目标网络匹配的接口 */
struct ifaddr *ifa_ifwithnet(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa, *ifa_maybe = NULL;
	struct sockaddr_dl *sdl;
	caddr_t cp, cp1, cp2, cplim;

	/* 被匹配的地址是数据链路曾地址时，直接返回传入地址中的接口索引对应的网络接口 */
	if(AF_LINK == addr->sa_family) {	
		sdl = (struct sockaddr_dl *)addr;
		if(sdl->sdl_index < ifnet_index)
			return ifnet_addrs[sdl->sdl_index]; 
	}

	list_for_each_entry(ifp, &ifnetlist, if_list) {
		list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) {
			if( ifa->ifa_addr->sa_family != addr->sa_family || \
					!ifa->ifa_netmask)
next:			continue;
			cp = addr->sa_data;	
			cp1 = ifa->ifa_addr->sa_data;	
			cp2 = ifa->ifa_netmask->sa_data;	
			cplim = (caddr_t)ifa->ifa_netmask + ifa->ifa_netmask->sa_len;
			while(cp2 < cplim) 
				if((*cp++ ^ *cp1++) & *cp2++)
					goto next;
			if(!ifa_maybe || rn_refines((caddr_t)ifa->ifa_netmask, (caddr_t)ifa_maybe->ifa_netmask))
				ifa_maybe = ifa;
		}
	}
	return ifa_maybe;
}

/* 查找本机地址存在匹配的接口（包括回环接口，P2P接口）或者存在广播地址匹配的接口 */
struct ifaddr *ifa_ifwithaddr(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;

	list_for_each_entry(ifp, &ifnetlist, if_list) {
		list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) {
			if( ifa->ifa_addr->sa_family != addr->sa_family || \
					!ifa->ifa_dstaddr)		//注意：这里要求查找的接口地址必须具有可回环性，很迷，IP地址均存在ifa_dstaddr
				continue;
			if(equal(addr, ifa->ifa_addr))
				return ifa;
			if((ifp->if_flags & IFF_BROADCAST) && \
					ifa->ifa_broadaddr && \
					equal(ifa->ifa_broadaddr, addr))
				return ifa;
		}
	}
	return NULL;
}

/**
 *  描述:接口遍历，并且对每一个接口执行回调函数
 *  返回：所有接口执行成功返回0，否则返回-1
 */
int if_callback_foreach(ifcallback_t func, void *arg)
{
	struct ifnet *ifp;

	list_for_each_entry(ifp, &ifnetlist, if_list) 
		if((*func)(ifp, arg))
			return -1;

	return 0;
}

/**
 *  描述:接口遍历，并且对状态为UP的接口执行回调函数
 *  返回：所有接口执行成功返回0，否则返回-1
 */
int if_callback_foreach_up(ifcallback_t func, void *arg)
{
	struct ifnet *ifp;

	list_for_each_entry(ifp, &ifnetlist, if_list) 
		if(ifp->if_flags & IFF_UP) 
			if((*func)(ifp, arg))
				return -1;

	return 0;
}

/**
 *  描述:接口遍历，并且对指定接口名的接口执行回调函数
 *  返回：所有接口执行成功返回0，否则返回-1
 */
int if_callback_foreach_name(ifcallback_t func, const char *ifname, void *arg)
{
	struct ifnet *ifp;

	list_for_each_entry(ifp, &ifnetlist, if_list) 
		if(0 == strcmp(ifp->if_xname, ifname))
			if((*func)(ifp, arg))
				return -1;

	return 0;
}

/***
 * 描述：根据指定的接口名称，查找匹配的ifnet结构
 * 返回：查找成功返回ifnet结构，失败返回NULL
 */

struct ifnet *if_find_byname(const char *ifname)
{
	struct ifnet *ifp;

	list_for_each_entry(ifp, &ifnetlist, if_list) {
		if(0 == strcmp(ifp->if_xname, ifname))
			return ifp;
	}

	return NULL;
}

int if_up(struct ifnet *ifp)
{
	//ifp->if_flags |= IFF_UP;		//将此处的状态改变放到具体的设备驱动ioctl函数中更为合适，此处仅进行ifnet通用处理
	return 0;
}

int if_down(struct ifnet *ifp)
{
	//ifp->if_flags &= ~IFF_UP;
	return 0;
}

/**
 * 描述：通用接口曾ioctl实现函数
 */
int if_ioctl(struct socket *so, unsigned long cmd, caddr_t arg)
{
	int s;
	struct ifnet *ifp;
	struct ifreq *ifr = (struct ifreq *)arg;
	struct ifconf *ifc = (struct ifconf *)arg;

	/* 获取系统所有接口信息 */
	if(SIOCGIFCONF == cmd)
		return if_getconf(ifc);

	if(NULL == (ifp = if_find_byname(ifr->ifr_name)))
		return ENODEV;

	switch(cmd) {
		case SIOCGIFFLAGS:
			ifr->ifr_flags = ifp->if_flags;	
			break;
		case SIOCGIFMETRIC:
			ifr->ifr_metric = ifp->if_metric;	
			break;
		/** 
 		 * 注意：该命令不会检查是否存在对已经置位的标识位进行重复置位的情况，因此可能发起不希望的ifioctl重复调用，
 		 * 需要上层调用者自身判断并检查重复性置位的问题，bad code 
 		 */
		case SIOCSIFFLAGS:
			if((ifr->ifr_flags & IFF_UP) && !(ifp->if_flags & IFF_UP)) {
				s = splimp();
				if_up(ifp);
				splx(s);
			}
			if(!(ifr->ifr_flags & IFF_UP) && (ifp->if_flags & IFF_UP)) {
				s = splimp();
				if_down(ifp);
				splx(s);
			}
			 ifr->ifr_flags &= ~IFF_CANTCHANGE;
			if(ifp->if_ioctl)
				return (*ifp->if_ioctl)(ifp, cmd, arg);
			break;
		case SIOCSIFMETRIC:
			ifp->if_metric = ifr->ifr_metric;	
			break;
		case SIOCGIFDATA:
			return copyout((caddr_t)ifr->ifr_data, (caddr_t)&ifp->if_data, sizeof(struct if_data));			
		case SIOCGETETHERADDR:
			if(ifp->if_ioctl)
				return (*ifp->if_ioctl)(ifp, cmd, arg);
			break;
		default:
			if(!so->soc_proto)
				return EOPNOTSUPP;
			return (*so->soc_proto->pr_usrreq)(so, PRU_CONTROL, (struct mbuf *)cmd, (struct mbuf *)arg, (struct mbuf *)ifp);
	}

	return 0;
}

/**
 * 描述：该函数将返回当前系统下所有网络接口名以及其地址信息
 * 参数：ifc，调用者将传入一个ifconf结构，当该结构的ifc_len为0时，该函数仅返回保存ifconf信息所需要的缓冲区长度，而不返回任何有效数据
 *  	 不为0时，该函数将返回ifc_len最大能够装下的网络接口信息
 */
int if_getconf(struct ifconf *ifc)
{
	int err = 0;
	ulong space;
	struct ifnet *ifp;
	struct ifaddr *ifa;
	struct sockaddr *sa;
	struct ifreq *ifrp;

	if(0 == (space = ifc->ifc_len)) {
		list_for_each_entry(ifp, &ifnetlist, if_list) { 
			if(!list_empty(&ifp->if_addrlist)) {
				list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) { 
					sa = ifa->ifa_addr;
					if(sa->sa_len > sizeof(struct sockaddr))
						space += sa->sa_len - sizeof(struct sockaddr);
					space += sizeof(struct ifreq);
				}
			} else {
				space += sizeof(struct ifreq);
			}
		}
		ifc->ifc_len = space;
		return 0;
	}

	ifrp = ifc->ifc_req; 
	list_for_each_entry(ifp, &ifnetlist, if_list) {
		if(space < sizeof(struct ifreq))
			break;
		if(!list_empty(&ifp->if_addrlist)) {
			list_for_each_entry(ifa, &ifp->if_addrlist, ifa_list) { 
				if(space < sizeof(struct ifreq))
					goto out;
				if(err = copyout(ifrp->ifr_name, ifp->if_xname, IFNAMESZ))
					goto out;
				sa = ifa->ifa_addr;
				if(sa->sa_len > sizeof(struct sockaddr)) {
					if((space - (sa->sa_len - sizeof(struct sockaddr))) < sizeof(struct ifreq))
						goto out;
					if(err = copyout((caddr_t)&ifrp->ifr_addr, (caddr_t)sa, sa->sa_len))
						goto out;
					ifrp = (struct ifreq *)((caddr_t)&ifrp->ifr_addr + sa->sa_len);
					space -= sa->sa_len - sizeof(struct sockaddr);
				} else {
					if(err = copyout((caddr_t)&ifrp->ifr_addr, (caddr_t)sa, sa->sa_len))
						goto out;
					++ifrp;
				}
				space -= sizeof(struct ifreq);
			}
		} else {
			bzero(&ifrp->ifr_addr, sizeof(struct sockaddr));
			++ifrp;
			space -= sizeof(struct ifreq);
		} 
	}

out:
	ifc->ifc_len -= space;		//更新ifc_req实际有效长度

	return err;
}


