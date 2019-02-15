
/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * Internet相关API 
 * 
 */

#ifndef __ARPA_INET_H__
#define __ARPA_INET_H__

extern in_addr_t inet_addr(const char *);				//defined in inet_addr.c
extern char *inet_ntoa(struct in_addr);					//defined in inet_addr.c
extern int inet_aton(const char *, struct in_addr *);	//defined in inet_addr.c	

#endif //__ARPA_INET_H__
