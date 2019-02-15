
/**
 * Copyright(c) 2017-8-4 Shangwen Wu	
 *
 * 网络路由相关定义
 * 
 */

#ifndef __NET_ROUTE_H__
#define __NET_ROUTE_H__

#include <net/radix.h>

struct sockaddr;
struct ifnet;
struct ifaddr;

/* 用于记录协议层的MTU以及路由中ARP表的生存时间等信息 */
struct rt_metrics {
	ulong rmx_mtu;				//协议的最大传输单元
	ulong rmx_expire;			//路由中ARP缓存表项的生存时间（某些情况下表示超时时间）
};

#define RTF_UP			0x0001			//表示当前路由表项有效
#define RTF_HOST		0x0002			//表示当前路由的下一跳地址指向一个具体的接口地址，而非一个网络
#define RTF_GATEWAY		0x0004			//表示当前路由为间接路由，其下一跳地址为中间地址，而非最终目的地址
#define RTF_REJECT		0x0008			//该字段，在ARP解析中被用到，用于暂时将路由挂起
#define RTF_CLONING		0x0010			//用于链路曾地址解析时标记将被CLONE的路由
#define RTF_LLINFO		0x0020			//用于标记专属路由（用于保存地址解析信息的clone路由）中已经分配了llinfo结构
#define RTF_PERMANENT_ARP	0x0040		//永久ARP，不允许修改

/* 路由表条目描述 */
struct rtentry {
	struct radix_node rt_nodes[2];		//一个作为叶子节点（必须为元素0），一个作为可能的叶子节点的父节点，rt_nodes必须位于其他成员之前
#define rt_key(rt)		((struct sockaddr *)((rt)->rt_nodes->rn_key))
#define rt_mask(rt)		((struct sockaddr *)((rt)->rt_nodes->rn_mask))
	struct ifnet *rt_ifp;				//指向该路由条目所在的网络接口
	struct ifaddr *rt_ifa;				
	struct sockaddr *rt_gateway;		//下一站网关（当路由为某一地址的专属路由时（CLONING路由）时，该字段将保存下一跳地址的链路地址信息）；该字段还用于保存链路地址信息
	struct sockaddr *rt_genmask;		//仅用于ARP地址接续使用
	struct rtentry *rt_gwroute;			//下一站网关的路由表项
	struct rtentry *rt_parent;			//用于专属路由指向其被clone的路由表项的指针
	void *rt_llinfo;					//路由表中保存的链路信息
	ulong rt_refcnt;					//引用计数，当该值为0时，才真正释放占用的内存空间
	uint16_t rt_flags;					//up/down
	struct rt_metrics rt_rmx;
#define rt_expire	rt_rmx.rmx_expire
#define rt_mtu		rt_rmx.rmx_mtu
};

/* 路由结构 */
struct route {
	struct rtentry *ro_rt;					//当前路由表项
	struct sockaddr ro_dst;					//当前路由对应的目的地址
};

/* 路由表操作 */
#define RTM_ADD			0x01	//添加一条路由
#define RTM_DELETE		0x02	//删除一条路由
#define RTM_RESOLVE		0x03	//用于解析链路曾地址时，增加一条clone路由

extern int rt_alloc(struct route *rt);		//defined in route.c
extern struct rtentry *__rt_alloc(struct sockaddr *dst, int report);
extern int rtinit(struct ifaddr *ifa, ulong cmd, int flags);
extern int rtrequest(struct sockaddr *, struct sockaddr *, struct sockaddr *, int, int, struct rtentry **);
extern struct ifaddr *rt_ifwithroute(struct sockaddr *dst, struct sockaddr *gateway, int flags);
extern int rt_setgate(struct rtentry *rt, struct sockaddr *dst, struct sockaddr *gate);
extern void rtredirect(struct sockaddr *dst, struct sockaddr *gateway, struct sockaddr *netmask, 
					int flags, struct sockaddr *src, struct rtentry **rtp);

#endif //__NET_ROUTE_H__
