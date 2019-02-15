
/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * ICMP协议实现 
 * 
 */
#ifndef __IP_ICMP_H__
#define __IP_ICMP_H__

struct in_addr;//defined in in.h
struct ip;	//defined in ip.h

/**
 * ICMP协议报文结构
 */
struct icmp {
	/* 以下8字节为ICMP共有字段 */
	u_int8_t icmp_type;
	u_int8_t icmp_code;
	u_int16_t icmp_cksum;
	/* 协议头部分 */
	union {
		/* 用于ICMP_PARAMPROB差错报文，该字段表示出错的IP字段相对与IP头的偏移位置 */
		uint8_t ih_pptr;
		/* 用于ICMP重定向报文，该字段保存重定向后的网关地址 */
		struct in_addr ih_gwaddr;
		/* 查询报文标识符字段 */ 
		struct ih_idseq {
			u_int16_t ids_id;
			u_int16_t ids_seq;
		} ih_idseq;
		/* 差错报文4个字节的zero字段 */
		u_int32_t ih_void;		
		/* 用于差错报文CODE=ICMP_UNREACH_NEEDFRAG */
		struct ih_pmtu {					
			u_int16_t ipm_void;
			u_int16_t ipm_nextpmtu;		
		} ih_pmtu;
	} icmp_hun;
#define icmp_pptr		icmp_hun.ih_pptr
#define icmp_gwaddr		icmp_hun.ih_gwaddr
#define icmp_id			icmp_hun.ih_idseq.ids_id
#define icmp_seq		icmp_hun.ih_idseq.ids_seq
#define icmp_void		icmp_hun.ih_void
#define icmp_pmvoid		icmp_hun.ih_pmtu.ipm_void
#define icmp_nextpmtu	icmp_hun.ih_pmtu.ipm_nextpmtu
	/* 数据部分 */
	union {
		/* 时间戳请求报文 */
		struct id_ts {
			u_int32_t its_otime;
			u_int32_t its_rtime;
			u_int32_t its_ttime;
		} id_ts;
		/* ICMP差错报文中，引起出错的IP头 */
		struct id_ip {
			struct ip idi_ip;
		} id_ip;
		/* 地址掩码查询报文 */
		u_int32_t id_mask;
		/* 其他数据 */
		u_int8_t id_data[1];
	} icmp_dun;
#define icmp_otime 		icmp_dun.id_ts.its_otime
#define icmp_rtime 		icmp_dun.id_ts.its_rtime
#define icmp_ttime 		icmp_dun.id_ts.its_ttime
#define icmp_ip			icmp_dun.id_ip.idi_ip
#define icmp_mask 		icmp_dun.id_mask
#define icmp_data 		icmp_dun.id_data
};

#define ICMP_MINLEN 	8							/* ICMP报文最小长度 */
#define ICMP_TSLEN 		(8 + 3 * sizeof(u_int32_t)) /* ICMP时间戳请求报文长度 */
#define ICMP_MASKLEN 	(8 + 4) 					/* ICMP地址掩码查询报文长度 */

#define ICMP_ADVLENMIN	(ICMP_MINLEN + sizeof(struct ip) + 8)	/* 最小ICMP差错报文长度 */
#define ICMP_ADVLEN(p)	(ICMP_MINLEN + ((p)->icmp_ip.ip_hlen << 2) + 8)	/* ICMP差错报文长度 */

/*
 * ICMP类型以及CODE字段定义 
 */
#define	ICMP_ECHOREPLY				0	/* echo reply */
#define	ICMP_UNREACH				3	/* dest unreachable, codes: */
#define	ICMP_UNREACH_NET			0	/* bad net */
#define	ICMP_UNREACH_HOST			1	/* bad host */
#define	ICMP_UNREACH_PROTOCOL		2	/* bad protocol */
#define	ICMP_UNREACH_PORT			3	/* bad port */
#define	ICMP_UNREACH_NEEDFRAG		4	/* IP_DF caused drop */
#define	ICMP_UNREACH_SRCFAIL		5	/* src route failed */
#define	ICMP_UNREACH_NET_UNKNOWN	6	/* unknown net */
#define	ICMP_UNREACH_HOST_UNKNOWN	7	/* unknown host */
#define	ICMP_UNREACH_ISOLATED		8	/* src host isolated */
#define	ICMP_UNREACH_NET_PROHIB		9	/* for crypto devs */
#define	ICMP_UNREACH_HOST_PROHIB	10	/* ditto */
#define	ICMP_UNREACH_TOSNET			11	/* bad tos for net */
#define	ICMP_UNREACH_TOSHOST		12	/* bad tos for host */
#define	ICMP_UNREACH_FILTER_PROHIB	13	/* prohibited access */
#define	ICMP_UNREACH_HOST_PRECEDENCE	14	/* precedence violat'n*/
#define	ICMP_UNREACH_PRECEDENCE_CUTOFF	15	/* precedence cutoff */
#define	ICMP_SOURCEQUENCH			4	/* packet lost, slow down */
#define	ICMP_REDIRECT				5	/* shorter route, codes: */
#define	ICMP_REDIRECT_NET			0	/* for network */
#define	ICMP_REDIRECT_HOST			1	/* for host */
#define	ICMP_REDIRECT_TOSNET		2	/* for tos and net */
#define	ICMP_REDIRECT_TOSHOST		3	/* for tos and host */
#define	ICMP_ECHO					8	/* echo service */
#define	ICMP_ROUTERADVERT			9	/* router advertisement */
#define	ICMP_ROUTERSOLICIT			10	/* router solicitation */
#define	ICMP_TIMXCEED				11	/* time exceeded, code: */
#define	ICMP_TIMXCEED_INTRANS		0	/* ttl==0 in transit */
#define	ICMP_TIMXCEED_REASS			1	/* ttl==0 in reass */
#define	ICMP_PARAMPROB				12	/* ip header bad */
#define	ICMP_PARAMPROB_OPTABSENT 	1	/* req. opt. absent */
#define	ICMP_TSTAMP					13	/* timestamp request */
#define	ICMP_TSTAMPREPLY			14	/* timestamp reply */
#define	ICMP_IREQ					15	/* information request */
#define	ICMP_IREQREPLY				16	/* information reply */
#define	ICMP_MASKREQ				17	/* address mask request */
#define	ICMP_MASKREPLY				18	/* address mask reply */

#define	ICMP_MAXTYPE				18

#define	ICMP_INFOTYPE(type) \
	((type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO || \
	(type) == ICMP_ROUTERADVERT || (type) == ICMP_ROUTERSOLICIT || \
	(type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY || \
	(type) == ICMP_IREQ || (type) == ICMP_IREQREPLY || \
	(type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY)

/* ICMP层调试开关 */
#define ICMPLVL_DEBUG		0
#if ICMPLVL_DEBUG
#define ICMPLVL_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define ICMPLVL_DBG(fmt, args...)		do{}while(0)	
#endif

struct ifnet;//defined in if.c

extern void icmp_error(struct mbuf *, int, int, ulong, struct ifnet *); //defined in ip_icmp.c
extern n_time iptime(void);//defined ip_icmp.c
extern void icmp_send(struct mbuf *, struct mbuf *);//defined in ip_icmp.c
extern void icmp_reflect(struct mbuf *);//defined in ip_icmp.c
extern int icmp_input(struct mbuf *m, ...);//defined in ip_icmp.c

#endif //__IP_ICMP_H__
