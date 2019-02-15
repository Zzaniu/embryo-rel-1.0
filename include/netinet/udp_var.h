
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * UDP协议相关定义
 * 
 */

#ifndef __NETINET_UDP_VAR_H__
#define __NETINET_UDP_VAR_H__

/* UDP层调试开关 */
#define UDPLVL_DEBUG		0
#if UDPLVL_DEBUG
#define UDPLVL_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define UDPLVL_DBG(fmt, args...)		do{}while(0)	
#endif

struct socket;
struct mbuf;

extern void udp_init(void);
extern int udp_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *n, struct mbuf *control);	
extern int udp_output(struct mbuf *m, ...);
extern int udp_input(struct mbuf *m, ...);

#endif //__NETINET_UDP_VAR_H__
