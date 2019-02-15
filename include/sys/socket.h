
/**
 * Copyright(c) 2017-2-21 PMON, Shangwen Wu	
 *
 * socket相关头文件
 * 
 */
#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

/* Types */
#define	SOCK_STREAM		1		/* stream socket */
#define	SOCK_DGRAM		2		/* datagram socket */
#define	SOCK_RAW		3		/* raw-protocol interface */
#define	SOCK_RDM		4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/* Address families.@PMON */
#define	AF_UNSPEC		0		/* unspecified */
#define	AF_LOCAL		1		/* local to host (pipes, portals) */
#define	AF_UNIX			AF_LOCAL	/* backward compatibility */
#define	AF_INET			2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK		3		/* arpanet imp addresses */
#define	AF_PUP			4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS		5		/* mit CHAOS protocols */
#define	AF_NS			6		/* XEROX NS protocols */
#define	AF_ISO			7		/* ISO protocols */
#define	AF_OSI			AF_ISO
#define	AF_ECMA			8		/* european computer manufacturers */
#define	AF_DATAKIT		9		/* datakit protocols */
#define	AF_CCITT		10		/* CCITT protocols, X.25 etc */
#define	AF_SNA			11		/* IBM SNA */
#define AF_DECnet		12		/* DECnet */
#define AF_DLI			13		/* DEC Direct data link interface */
#define AF_LAT			14		/* LAT */
#define	AF_HYLINK		15		/* NSC Hyperchannel */
#define	AF_APPLETALK	16		/* Apple Talk */
#define	AF_ROUTE		17		/* Internal Routing Protocol */
#define	AF_LINK			18		/* Link layer interface */
#define	pseudo_AF_XTP	19		/* eXpress Transfer Protocol (no AF) */
#define	AF_COIP			20		/* connection-oriented IP, aka ST II */
#define	AF_CNT			21		/* Computer Network Technology */
#define pseudo_AF_RTIP	22		/* Help Identify RTIP packets */
#define	AF_IPX			23		/* Novell Internet Protocol */
#define	AF_INET6		24		/* IPv6 */
#define pseudo_AF_PIP	25		/* Help Identify PIP packets */
#define AF_ISDN			26		/* Integrated Services Digital Network*/
#define AF_E164			AF_ISDN		/* CCITT E.164 recommendation */
#define AF_NATM			27		/* native ATM access */
#define	AF_ENCAP		28
#define	AF_SIP			29		/* Simple Internet Protocol */
#define AF_KEY			30
#define	AF_MAX			31

/* socket选项设置 */
#define SO_REUSEADDR	0x0004				/* 本地地址可重用 */
#define SO_ACCEPTCONN	0x0002				/* socket已经调用listen进行监听 */
#define SO_DONTROUTE	0x0010				/* 关闭地址路由发送网络报文到指定接口地址，TTL为1 */
#define SO_BROADCAST	0x0020				/* socket是否支持广播 */
#define SO_REUSEPORT	0x0200				/* 本地地址和端口可重用 */

/* 通用网络地址结构 */
struct sockaddr {
	unsigned char 	sa_len;					//结构总长度
	sa_family_t 	sa_family;				//网络域
	char 			sa_data[14];			//其他数据，一般该数据根据实际使用域不同而不同
};

struct iovec;		//defined in uio.h

/* 封装用户网络数据信息的结构体 */
struct msghdr {
	caddr_t msg_soname;						//sockaddr地址
	ulong msg_sonamelen;					//sockaddr长度
	caddr_t msg_control;					//控制信息
	socklen_t msg_controllen;				//控制信息长度
	struct iovec *msg_iov;					//IO集合基地值
	ulong msg_iovcnt;						//IO集合数
	uint msg_flags;							//接收消息标识
};


/* socket消息的操作标识位 */
#define MSG_DONTWAIT	0x0001
#define MSG_EOR			0x0002
#define MSG_DONTROUTE	0x0004
#define MSG_CTRUNC		0x0008
#define MSG_OOB			0x0010
#define MSG_PEEK		0x0020
#define MSG_WAITALL		0x0040
#define MSG_BCAST		0x0080
#define MSG_MCAST		0x0100
#define MSG_TRUNC		0x0200

#define SCM_RIGHTS		0x01

/* 控制信息结构 */
struct cmsghdr {
	socklen_t cmsg_len;	/* 控制信息总长度，包括数据部分和头部 */
	int cmsg_level;		/* 源协议 */
	int cmsg_type; 		/* 协议相关类型 */
	/* next is uchar cmsg_data[] */
};

/* 根据控制信息首部返回数据部分 */
#define CMSG_DATA(cmsg)	((uchar *)((cmsg) + 1))


extern int socket(int, int, int);
extern int bind(int, const struct sockaddr *, socklen_t);
extern ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
extern ssize_t recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);

#endif	//__SYS_SOCKET_H__
