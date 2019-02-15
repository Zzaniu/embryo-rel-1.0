
/**
 * Copyright(c) 2017-2-23 Shangwen Wu	
 *
 * domain相关头文件
 * 
 */

#ifndef __SYS_DOMAIN_H__
#define __SYS_DOMAIN_H__

struct mbuf;							//defined in mbuf.h
struct protocol;						//defined in protocol.h
struct sockaddr;						//defined in socket.h

/* 添加一个名为“XXXdomaon”的domain到domains链表中 */
#define ADDDOMAIN(name)								\
do {													\
	extern struct domain __CONCAT(name, domain); 	\
	__CONCAT(name, domain).dom_next = domains;		\
	domains = &__CONCAT(name, domain);				\
}while(0)

/* 网络域结构定义 */
struct domain {
	u_int8_t dom_family;				/* 域代码，同socket传入的AF_XXX */
	const char *dom_name;				/* 域名称 */
	struct protocol *dom_proto, *dom_proto_last;	/* 当前域包含的协议集合 */
	struct domain *dom_next;			/* 域结构链表 */
	void (*dom_init)(void);				/* 初始化方法 */
	int (*dom_rtattach)(void **, int);	/* 用于初始化当前域的radix_node_head */
	int (*dom_externalize)(struct mbuf *);	/* */
	int dom_rtoff;						/* 初始化radix_node_head的起始偏移bit数 */
	int dom_maxrtkey;					/* 当前域radix_node的最大key长度 */
};

extern struct domain *domains;			//defined in uipc_domain.c

extern void domaininit(void);			//defined in uipc_domain.c
extern struct protocol *profindbyproto(u_int8_t, \
			u_int16_t, u_int16_t);		//defined in uipc_domain.c
extern struct protocol *profindbytype(u_int8_t, \
			u_int16_t);		//defined in uipc_domain.c
extern void pfctlinput(int cmd, struct sockaddr *sa);//defined in uipc_domain.c

#endif //__SYS_DOMAIN_H__

