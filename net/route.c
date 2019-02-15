
/**
 * Copyright(c) 2017-8-16 Shangwen Wu	
 *
 * 路由相关
 */

#include <common.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/system.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/domain.h>
#include <sys/protocol.h>
#include <mach/intr.h>

#include <net/if.h>
#include <net/if_type.h>
#include <net/route.h>

/* route调试开关 */
#define ROUTE_DEBUG		0
#if ROUTE_DEBUG
#define ROUTE_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define ROUTE_DBG(fmt, args...)		do{}while(0)	
#endif

struct radix_node_head *rt_table[AF_MAX + 1] = {0};

/**
 * 描述：路由重定向
 */
void rtredirect(struct sockaddr *dst, struct sockaddr *gateway, struct sockaddr *netmask, 
					int flags, struct sockaddr *src, struct rtentry **rtp)
{

}

/**
 * 描述：路由表操作函数，该函数一般用于配置接口时的路由操作，而不用于route操作命令
 * 参数
 */
int rtinit(struct ifaddr *ifa, ulong cmd, int flags)
{
	int ret = 0;
	struct rtentry *rt, *nrt;
	struct sockaddr *dst;

	/* 
 	 * 当RTF_HOST置位时，当前接口应当为P2P或者回环接口，因此路由表的“目标地址”应当是该接口的对端地址 
 	 * 没有置位时，dst为当前设置的本地接口地址，因为最终的“目标地址”为本地地址所在的（子）网络
 	 */
	dst = flags & RTF_HOST ? ifa->ifa_dstaddr : ifa->ifa_addr;

	/* 
  	 * 注意：这里传入的gateway为本地地址，原因是该函数添加的路由RTF_GATEWAY不会被设置，因此根据路由表
  	 * 要求，这里传入当前接口的本地地址
     */
	ret = rtrequest(dst, ifa->ifa_addr, ifa->ifa_netmask, ifa->ifa_flags | flags, cmd, &nrt);
		
	/* 
	 * 判断返回路由表项关联的ifaddr是否为我们当前设置的ifaddr，如果不是需要删除原有ifaddr的路由，
 	 * 而添加新的路由设置到当前的ifaddr
 	 */
	if(RTM_ADD == cmd && !ret && (rt = nrt) != NULL) {
		rt->rt_refcnt--;			//rtrequest将持有路由，而这里并不需要持有路由，所以令refcnt--
		if(rt->rt_ifa != ifa) {
			log(LOG_ERR, "rtinit: wrong ifaddr %p was %p\n", ifa, rt->rt_ifa);
			if(rt->rt_ifa->ifa_rtrequest)
				(*rt->rt_ifa->ifa_rtrequest)(RTM_DELETE, rt, NULL);
			//bad code
			//ifafree(rt->rt_ifa);
			rt->rt_ifa = ifa;
			rt->rt_ifp = ifa->ifa_ifp;
			ifa->ifa_refcnt++;
			if(ifa->ifa_rtrequest)
				(*ifa->ifa_rtrequest)(RTM_ADD, rt, NULL);
		}
	}

	return ret;
}

/**
 * 描述;该函数用于确定将数据报发往下一站路由网关的处理接口（对于直接路由就是出口地址对应的接口）
 */
struct ifaddr *rt_ifwithroute(struct sockaddr *dst, struct sockaddr *gateway, int flags)
{
	struct rtentry *rt;
	struct ifaddr *ifa = NULL, *oifa;

	if(!(flags & RTF_GATEWAY)) {
		/*
 		 * 当一个路由的下一站网关为一个主机时，说明最终目的地与当前系统的接口直接相联
 		 * ，那么当路由目的地恰好又表示一个主机地址时，dst地址就有可能是一个P2P接口的
 		 * 对端地址，下面对这样一种情形进行判断
 		 */ 
		if(flags & RTF_HOST)
			ifa = ifa_ifwithdstaddr(dst);
		/*
 		 * 如果最终目的地与当前系统直接相联，那么gateway应当是系统某一个接口的本地地
 		 * 址，下面用于定位到这个接口
 		 */
		if(NULL == ifa)
			ifa = ifa_ifwithaddr(gateway);
	} else {
		/*
 		 * 当一个路由的下一站网关为一个路由器时，说明最终目的地与当前系统的接口没有直
 		 * 接相联，这种情况下，gateway可能是当前系统下某一个P2P接口的对端，因此这里也
 		 * 需要对这种情况作出判断
 		 */ 
		ifa_ifwithdstaddr(gateway);
	}
	/* 如果以上步骤没有找到合适的P2P接口，下面将查找与gateway位于同一网络的接口 */
	if(NULL == ifa)
		ifa_ifwithnet(gateway);

	/* 如果还有没有找到合适的接口，那么将查找路由表中的匹配路径 */
	if(NULL == ifa) {
		rt = __rt_alloc(gateway, 0);
		if(!rt)
			return NULL;
		rt->rt_refcnt--;			//立即释放引用，因为这里并不持有路由
		/* 如果ifa与dst位于同一网络域，那么下一站网关必须与当前系统直接相联，因此该路由不能为RTF_GATEWAY */
		if((rt->rt_flags & RTF_GATEWAY) && (rt_key(rt)->sa_family == dst->sa_family))
			return NULL;
		if(!(ifa = rt->rt_ifa))
			return NULL;
	}
	/* 对于目的地址与定位到的接口地址不在同一网络域的情况，将尝试查找该接口下其他位于同一网络域的接口 */
	if(ifa->ifa_addr->sa_family != dst->sa_family) {
		oifa = ifa;
		if(!(ifa = ifaof_ifpforaddr(dst, ifa->ifa_ifp)))
			ifa = oifa;
	}

	return ifa;
}

/**
 * 描述：为路由表项设置gateway，当空间不足时，将分配更多空间，并且该函数还将对gateway获取路由
 * 注意：该函数同时也将为key分配一段内存空间
 */
int rt_setgate(struct rtentry *rt, struct sockaddr *dst, struct sockaddr *gate)
{
	ulong glen, dlen;
	caddr_t old, new;

	glen = ROUNDUP(gate->sa_len);
	dlen = ROUNDUP(dst->sa_len);

	if(!rt->rt_gateway || glen > ROUNDUP(rt->rt_gateway->sa_len)) {
		if(!(new = kmem_malloc(glen + dlen)))
			return -1;
		old = (caddr_t)rt_key(rt);
		rt->rt_nodes->rn_key = new;
	} else {
		new = (caddr_t)rt_key(rt);
		old = NULL;
	}

	bcopy((caddr_t)gate, new + dlen, glen);
	rt->rt_gateway = (struct sockaddr *)(new + dlen);
	if(old) {
		bcopy((caddr_t)old, new, dlen);
		kmem_free(old);
	}

	if(rt->rt_gwroute) {
		;//rtfree(rt->rt_gwroute);
		rt->rt_gwroute = NULL;
	}

	if(rt->rt_flags & RTF_GATEWAY) 
		rt->rt_gwroute = __rt_alloc(rt->rt_gateway, 1);

	return 0;
}

/**
 * 描述：基于掩码的地址拷贝
 */
void rt_maskedcopy(struct sockaddr *src, struct sockaddr *dst, struct sockaddr *netmask)
{
	caddr_t cp, cp2, cp3, cplim, cplim2;
	
	cp = (caddr_t)src;
	cp2 = (caddr_t)dst;
	cp3 = (caddr_t)netmask;
	cplim = cp2 + *cp3;
	cplim2 = cp2 + *cp;

	/* 拷贝sa_family以及sa_len */
	*cp2++ = *cp++;
	*cp2++ = *cp++;
	cp3 += 2;

	if(cplim > cplim2)
		cplim = cplim2;

	while(cp2 < cplim)
		*cp2++ = *cp++ & *cp3++;

	if(cp2 < cplim2)
		bzero(cp2, cplim2 - cp2);

}

/**
 * 描述：路由表项操作的底层函数
 * 参数：dst，路标表项的目的地址；gateway，下一站网关地址；netmask，掩码；flags，标志位；ret_rt，返回查找到的路由表项结构
 */
int rtrequest(struct sockaddr *dst, struct sockaddr *gateway, 
			struct sockaddr *netmask, int flags, int req, struct rtentry **ret_rt)
{
	int err = 0;
	struct ifaddr *ifa;
	struct rtentry *rt = NULL;
	struct radix_node_head *rnh = rt_table[dst->sa_family];
	int s = splsoftnet();
	struct sockaddr *ndst;
	struct radix_node *rn;
#define senderr(x) do {err = (x); goto failed;} while(0)

	if(NULL == rnh) {
		log(LOG_DEBUG, "rtrequest: cant find radix_node_head by family %d\n", 
				dst->sa_family);
		senderr(EAFNOSUPPORT);
	}

	if(flags & RTF_HOST)
		netmask = NULL;

	switch(req) {
	case RTM_RESOLVE:
		/* 要求被clone的路由必须不为空 */
		if(!ret_rt || !*ret_rt)
			senderr(EINVAL);
		ifa = (*ret_rt)->rt_ifa;
		/* 清楚clone标志，以免重复创建专属路由 */
		flags = (*ret_rt)->rt_flags & ~RTF_CLONING;
		/* 除非通过rtsock设置，否则netmask一般为NULL，表示专属路由为一个主机路由 */
		if(!(netmask = (*ret_rt)->rt_genmask))
			flags |= RTF_HOST;
		gateway = (*ret_rt)->rt_gateway;	
		goto makeroute;
	case RTM_ADD:
		if(!(ifa = rt_ifwithroute(dst, gateway, flags)))
			senderr(ENETUNREACH);

makeroute:
		if(!(rt = (struct rtentry *)kmem_zmalloc(sizeof(*rt))))
			senderr(ENOBUFS);
		rt->rt_flags = flags;
		if(rt_setgate(rt, dst, gateway)) {
			kmem_free(rt);
			senderr(ENOBUFS);
		}
		/* 注意：radix节点中保存的key是进行掩码与之后的值，并非完整的dst地址 */
		ndst = rt_key(rt);
		if(netmask)
			rt_maskedcopy(dst, ndst, netmask);
		else 
			bcopy((caddr_t)dst, (caddr_t)ndst, dst->sa_len);
		/* 添加radix树节点 */			
		if(!(rn = rnh->rnh_addaddr((caddr_t)ndst, (caddr_t)netmask, rnh, rt->rt_nodes))) {
			if(rt->rt_gwroute)
				;//rtfree(rt->rt_gwroute); bad code
			kmem_free(rt_key(rt));
			kmem_free(rt);
			senderr(EEXIST);	
		}
		ifa->ifa_refcnt++;
		rt->rt_ifa = ifa;
		rt->rt_ifp = ifa->ifa_ifp;
		if(RTM_RESOLVE == req){
			rt->rt_rmx = (*ret_rt)->rt_rmx;
			rt->rt_parent = *ret_rt;
		}
		if(ifa->ifa_rtrequest)
			ifa->ifa_rtrequest(req, rt, (struct sockaddr *)(ret_rt ? *ret_rt : NULL));
		if(ret_rt) {
			rt->rt_refcnt++;//初始化计数值为1
			*ret_rt = rt;
		}

		break;
	}
	
failed:
	splx(s);

	return err;
}

/**
 * 描述：查找radix路由树，若查找成功将返回位域radix树中的rtentry指针，反之则清空该指针
 * 参数：dst用于查找匹配的目的地址；report用于进行地址resolve时，是否进行错误信息汇报
 * 注意：该函数在查找到一个匹配的路由表项后，将会增加该路由表项的引用计数
 */
struct rtentry *__rt_alloc(struct sockaddr *dst, int report)
{
	struct rtentry *rt, *ret_rt = NULL;
	struct radix_node *rn;
	int s = splnet();
	struct radix_node_head *rnh = rt_table[dst->sa_family];

	if(NULL == rnh) {
		log(LOG_DEBUG, "rt_alloc: cant find radix_node_head by family %d\n", 
				dst->sa_family);
		splx(s);
		return NULL;
	}

	if((rn = rnh->rnh_match(dst, rnh)) \
			&& !(rn->rn_flags & (RNF_ROOT))) {
		ret_rt = rt = (struct rtentry *)rn;
		if(report && (rt->rt_flags & RTF_CLONING)) {
			/** 
 			 * 当路由为clone类型时，需要通过rtrequest重新创建一条专门用于解析链路地址的路由表项，
 			 * 而该新创建的专属于某个特定地址的主机路由将会在本次和下一次调用__rt_alloc查找路由时
 			 * 优先得到匹配，以避免重复创建一模一样的路由，并且方便调用者直接得到包含链路地址信息
 			 * 的地址路由
 			 */
			if(rtrequest(dst, NULL, NULL, 0, RTM_RESOLVE, &ret_rt)) {
				//resolve faild, report it!
				++rt->rt_refcnt;//专属路由分配失败时，使用原来的路由，这里将原路由计数加1
				ret_rt = rt;
				goto miss;
			}
		} else
			ret_rt->rt_refcnt++;
	} else {
		ROUTE_DBG("rt_alloc: no matched node\n"); 
	}

miss:
	/* bad code 这里需要添加错误汇报处理 */

	splx(s);

	return ret_rt;
}
	
int rt_alloc(struct route *ro)
{
	if(ro->ro_rt)
		return 0;
	if((ro->ro_rt = __rt_alloc(&ro->ro_dst, 1)) != NULL)
		return 0;

	return -1;
}

/* 初始化路由域的radix树 */
static void route_init(void)
{
	struct domain *dp;
	extern struct domain *domains;					
	
	/* 初始化一个全局的掩码radix树 */
	rn_init();

	/* 初始化其他所有域的radix树 */
	for(dp = domains; dp != NULL; dp = dp->dom_next) 
		if(dp->dom_rtattach)
			(*dp->dom_rtattach)((void **)(&rt_table[dp->dom_family]), dp->dom_rtoff);
}

extern struct domain routedomain;

/* Internet协议集合，注意，同一类型的协议的init函数只能被调用一次 */
static struct protocol route_protos[] = {
	{SOCK_RAW, 0, PRF_ATOMIC, &routedomain, NULL, NULL, NULL},
};

/* Internet域定义 */
struct domain routedomain = 
{
	.dom_family = AF_ROUTE,
	.dom_name = "Route",
	.dom_proto = route_protos,
	.dom_proto_last = route_protos + NR(route_protos),
	.dom_next = NULL,
	.dom_init = route_init,
	.dom_rtattach = NULL,
	.dom_rtoff = 0,
	.dom_maxrtkey = 0,
};

