
/**
 * Copyright(c) 2017-2-23 Shangwen Wu	
 *
 * protocol相关头文件
 * 
 */
#ifndef __SYS_PROTOCOL_H__
#define __SYS_PROTOCOL_H__

struct mbuf;					//defined in mbuf.h
struct domain;					//defined in domain.h
struct socket;					//defined in socketvar.h
struct sockaddr;				//defined in socket.h


/* 用于protocol.pr_usrreq函数的命令参数 */
#define PRU_ATTACH			0	/* 用于socket函数，初始化协议相关资源 */
#define PRU_SEND			1	/* 发送数据请求 */
#define PRU_CONTROL			2	/* 协议曾ioctl */
#define PRU_RCVOOB			3	/* 接收OOB数据 */
#define PRU_RCVD			4	/* 数据已被上层取出，缓冲区空间空闲通知，用于TCP等流式协议 */
#define PRU_BIND			5	/* 绑定本地地址和端口 */
#define PRU_DETACH			6	/* 释放协议相关资源 */

#define PRF_ATOMIC			0x0001	/* 协议要求原子发送整个数据包 */
#define PRF_CONNREQUIRED	0x0002	/* 该协议面向连接 */
#define PRF_ADDR			0x0004	/* 协议要求报文中包含地址 */
#define PRF_WANTRCVD		0x0008	/* 协议要求RCVD通报 */


/* 用于ctlinput函数的命令参数 */
#define PRC_UNREACH_NET			0	/* 没有到指定网络的路由 */
#define PRC_MSGSIZE				1	/* 消息长度限制导致丢包 */
#define PRC_UNREACH_HOST		2	/* 没有到指定主机的路由 */
#define PRC_TIMXCEED_INTRANS	3	/* 传输时间超过了包的生存时间 */
#define PRC_PARAMPROB			4	/* 协议头出错 */
#define PRC_QUENCH				5	/* ？？？ */
#define PRC_REDIRECT_HOST		6	/* 主机路由重定向 */

struct protocol { 
	short pr_type;				/* 协议类型，同socket参数SOCK_XXX */	
	short pr_protocol;			/* 协议编号，同socket参数IPPROTO_XXX */	
	short pr_flags;			/* 标识字段 */
	struct domain *pr_domain;	/* 指向本协议所属的域 */
	void (*pr_init)(void);		/* 协议初始化方法 */
	int (*pr_usrreq)(struct socket *, int, struct mbuf *, struct mbuf *, struct mbuf *);	/* 用户协议请求处理方法 */
	int (*pr_output)(struct mbuf *, ...);	/* 协议曾输出函数 */
	int (*pr_input)(struct mbuf *, ...);	/* 协议曾输入函数 */
	void *(*pr_ctlinput)(int, struct sockaddr *, void *);/* 下层协议的特殊处理 */
};

#endif //__SYS_PROTOCOL_H__

