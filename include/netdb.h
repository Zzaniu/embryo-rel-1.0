
/**
 * Copyright(c) 2018-10-09 Shangwen Wu	
 *
 * 用于internet网络服务相关的信息检索
 * 
 */

#ifndef __NETDB_H__
#define __NETDB_H__

struct servent {
	char *s_name;			/* 服务名 */
	char **s_aliases;		/* 别名 */
	int s_port;				/* 知名端口号 */
	char *s_proto;			/* TCP or UDP */
};

extern struct servent *getservbyname(const char *name, const char *proto);	//defined in getservbyname.c
extern struct servent *getservent(void);	//defined in getservent.c
extern void setservent(int);
extern void endservent(void);

#endif //__NETDB_H__
