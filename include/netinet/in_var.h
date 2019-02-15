
/**
 * Copyright(c) 2017-8-4 Shangwen Wu	
 *
 * Internet相关定义
 * 
 */

#ifndef __NETINET_IN_VAR_H__
#define __NETINET_IN_VAR_H__

struct socket;
struct ifnet;
struct ifaddr;
struct sockaddr_in;
struct list_head;

/* Internet域接口地址描述 */
struct in_ifaddr {
	struct ifaddr ia_ifa;
#define ia_ifp		ia_ifa.ifa_ifp
#define ia_flags	ia_ifa.ifa_flags
	in_addr_t ia_net;					//网络号
	in_addr_t ia_netmask;				//网络掩码
	in_addr_t ia_subnet;				//子网号
	in_addr_t ia_subnetmask;			//子网掩码
	struct sockaddr_in ia_addr;			//ifaddr->ifa_addr实际存储位置
	struct sockaddr_in ia_dstaddr;		//ifaddr->ifa_dstaddr实际储存位置
#define ia_broadaddr ia_dstaddr			//基于子网的广播
	struct sockaddr_in ia_sockmask;		//ifaddr->ifa_netmask实际存储位置
	struct list_head ia_list;			//in_ifaddr地址节点
	struct list_head ia_multlist;		//多播地址链表
	struct in_addr ia_netbroadcast;		//基于网络的广播
};

struct in_ifaliasreq {
	char iifru_name[IFNAMESZ];
	struct sockaddr_in iifar_addr;
	struct sockaddr_in iifar_dstaddr;
#define iifar_broadaddr iifar_dstaddr
	struct sockaddr_in iifar_mask;
};

#define ifatoia(ifa)		((struct in_ifaddr *)(ifa))

extern int in_control(struct socket *so, ulong cmd, void *data, struct ifnet *ifp); //defined in in.c
extern int in_ifinit(struct ifnet *, struct in_ifaddr *, struct sockaddr_in *, int);
extern void in_ifscrub(struct ifnet *, struct in_ifaddr *);
extern void in_socktrim(struct sockaddr_in *sin);
extern int in_broadcast(struct in_addr addr);

#endif //__NETINET_IN_VAR_H__
