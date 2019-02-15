
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * Ip协议相关定义
 * 
 */

#ifndef __NETINET_IP_VAR_H__
#define __NETINET_IP_VAR_H__

struct socket;
struct mbuf;

/* IP部分选项 */
#define IP_FORWARDING		0x01
#define IP_RAWOUTPUT		0x02
#define IP_ROUTETOIF		SO_DONTROUTE
#define IP_ALLOWBROADCAST	SO_BROADCAST


#define MAX_IPOPTLEN		40

/* IP选项定义 */
struct ip_option {
	struct in_addr ipopt_dst;
	uchar ipopt_list[MAX_IPOPTLEN];
};

extern struct ifqueue ipintrq;	//defined in ip_input.c

/* IP层调试开关 */
#define IPLVL_DEBUG		0
#if IPLVL_DEBUG
#define IPLVL_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define IPLVL_DBG(fmt, args...)		do{}while(0)	
#endif

extern void rip_init(void); //defined in raw_ip.c
extern void ip_init(void); //defined in ip_input.c
extern int rip_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *n, struct mbuf *control); //defined in raw_ip.c
extern int rip_output(struct mbuf *m, ...); //defined in raw_ip.c
extern int rip_input(struct mbuf *m, ...);//defined in raw_ip.c
extern int ip_output(struct mbuf *m, ...); //defined ip_output.c
extern void ipintr(void); //defined in ip_input.c
extern int ipv4_input(struct mbuf *m, ...);//defined in ip_input.c
extern int ip_dooptions(struct mbuf *m);//defined ip_input.c
extern struct in_ifaddr *ip_rtaddr(struct in_addr dst);//defined in ip_input.c
extern void ip_forward(struct mbuf *m, int srcrt);//defined in ip_input.c
extern struct in_ifaddr *in_iawithaddr(struct in_addr ina, struct mbuf *m);//defined in ip_input.c
extern struct mbuf *ip_srcroute(void);//defined in ip_input.c
extern void ip_stripoptions(struct mbuf *m, struct mbuf *mo);//defined in ip_input.c

#endif //__NETINET_IP_VAR_H__
