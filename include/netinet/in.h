
/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * Internet相关定义
 * 
 */

#ifndef __NETINET_IN_H__
#define __NETINET_IN_H__

typedef u_int32_t in_addr_t;
typedef u_int16_t in_port_t;

/* Protocols @PMON */
#define	IPPROTO_IP			0		/* dummy for IP */
#define IPPROTO_HOPOPTS		IPPROTO_IP	/* Hop-by-hop option header. */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_IGMP		2		/* group mgmt protocol */
#define	IPPROTO_GGP			3		/* gateway^2 (deprecated) */
#define	IPPROTO_IPIP		4		/* IP inside IP */
#define	IPPROTO_IPV4		IPPROTO_IPIP	/* IP inside IP */
#define	IPPROTO_TCP			6		/* tcp */
#define	IPPROTO_EGP			8		/* exterior gateway protocol */
#define	IPPROTO_PUP			12		/* pup */
#define	IPPROTO_UDP			17		/* user datagram protocol */
#define	IPPROTO_IDP			22		/* xns idp */
#define	IPPROTO_TP			29 		/* tp-4 w/ class negotiation */
#define IPPROTO_IPV6		41		/* IPv6 in IPv6 */
#define IPPROTO_ROUTING		43		/* Routing header. */
#define IPPROTO_FRAGMENT	44		/* Fragmentation/reassembly header. */
#define	IPPROTO_ESP			50		/* Encap. Security Payload */
#define	IPPROTO_AH			51		/* Authentication header */
#define IPPROTO_ICMPV6		58		/* ICMP for IPv6 */
#define IPPROTO_NONE		59		/* No next header */
#define IPPROTO_DSTOPTS		60		/* Destination options header. */
#define	IPPROTO_EON			80		/* ISO cnlp */
#define	IPPROTO_ENCAP		98		/* encapsulation header */
#define	IPPROTO_RAW			255		/* raw IP packet */

#define	IPPROTO_MAX			256

/* IPV4网络地址结构 */
struct in_addr { 
	in_addr_t s_addr;
};

struct sockaddr_in {
	u_int8_t 		sin_len;				//结构总长度
	sa_family_t 	sin_family;				//网络域
	/* 后续长度必须要和sockaddr.sa_data长度一样 */
	in_port_t		sin_port;				//TCP/IP端口
	struct in_addr 	sin_addr;
	u_int8_t		sin_zero[8];		
};

/* IPV6网络地址结构 */
struct in6_addr { 
	u_int8_t s6_addr[16];
};

struct sockaddr_in6 {
	u_int8_t 		sin6_len;				//结构总长度
	sa_family_t 	sin6_family;			//网络域
	/* 后续长度必须要和sockaddr.sa_data长度一样 */
	in_port_t		sin6_port;				//TCP/IP端口
	u_int32_t 		sin6_flowinfo;	
	struct in6_addr sin6_addr;
	u_int32_t		sin6_scope_id;
};

#define satosin(sa)			((struct sockaddr_in *)(sa))
#define sintosa(sin)		((struct sockaddr *)(sin))

#define __IPADDR(x)	((u_int32_t)htonl((u_int32_t)(x)))

#define INADDR_ANY			__IPADDR(0x00000000)
#define INADDR_LOOPBACK		__IPADDR(0x7f000001)
#define INADDR_BROADCAST	__IPADDR(0xffffffff)
#define INADDR_NONE			__IPADDR(0xffffffff)

#define IN_CLASSA(x)		(((uint32_t)(x) & __IPADDR(0x80000000)) == __IPADDR(0x00000000))
#define IN_CLASSB(x)		(((uint32_t)(x) & __IPADDR(0xc0000000)) == __IPADDR(0x80000000))
#define IN_CLASSC(x)		(((uint32_t)(x) & __IPADDR(0xe0000000)) == __IPADDR(0xc0000000))
#define IN_CLASSD(x)		(((uint32_t)(x) & __IPADDR(0xf0000000)) == __IPADDR(0xe0000000))
#define IN_CLASSE(x)		(((uint32_t)(x) & __IPADDR(0xf8000000)) == __IPADDR(0xf0000000))

#define IN_CLASSA_MASK		__IPADDR(0xff000000)
#define IN_CLASSB_MASK		__IPADDR(0xffff0000)
#define IN_CLASSC_MASK		__IPADDR(0xffffff00)
#define IN_CLASSD_MASK		__IPADDR(0xf0000000)

#define IN_CLASSA_NSHIFT	24
#define IN_CLASSB_NSHIFT	16
#define IN_CLASSC_NSHIFT	8
#define IN_CLASSD_NSHIFT	28

#define IN_CLASSA_NET(x)	(((uint32_t)(x) & IN_CLASSA_MASK))
#define IN_CLASSB_NET(x)	(((uint32_t)(x) & IN_CLASSB_MASK))
#define IN_CLASSC_NET(x)	(((uint32_t)(x) & IN_CLASSC_MASK))
#define IN_CLASSD_NET(x)	(((uint32_t)(x) & IN_CLASSD_MASK))

#define IN_MULTICAST(x)		IN_CLASSD(x)
#define IN_EXPERIMENTAL(x)	(((uint32_t)(x) & __IPADDR(0xf0000000)) == __IPADDR(0xf0000000))

#define IN_LOOPBACKNET		127

/* 端口号分配区间 */
#define IPPORT_RESERVED			1024
#define IPPORT_USERRESERVED		49151
#define IPPORT_HIFIRSTAUTO		49152
#define IPPORT_HILASTAUTO		65535

struct mbuf;

extern int in_cksum(struct mbuf *m, int len); //define in_cksum.c

extern struct list_head in_ifaddrlist;	//defined in.c

extern int in_canforward(struct in_addr ina); //defined in.c

#endif //__NETINET_IN_H__
