
/**
 * Copyright(c) 2017-2-23 Shangwen Wu 
 *
 * 网络域处理 
 * 
 */
#include <common.h>
#include <cdef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/domain.h>
#include <sys/protocol.h>
#include <netinet/in.h>

struct domain *domains = NULL;					//全局domain链表指针
ulong max_hdr = 0;								//最大协议头+link长度
ulong max_linkhdr = 0;							//最大链路层包头长度
ulong max_protohdr = 0;							//最大链路层以上的数据包头长度

/**
 * 描述：每个协议均执行一次ctlinput
 */
void pfctlinput(int cmd, struct sockaddr *sa)
{
	struct domain *dp;
	struct protocol *pp;

	for(dp = domains; dp != NULL; dp = dp->dom_next)
		for(pp = dp->dom_proto; pp < dp->dom_proto_last; ++pp) 
			if(pp->pr_ctlinput)
				(*pp->pr_ctlinput)(cmd, sa, NULL);
}

/**
 * 描述：添加当前BIOS支持的各种网络域，并调用域以及各个域包括协议的初始化方法
 */
void domaininit(void)
{
	struct domain *dp;
	struct protocol *pp;
	
	/* 将BIOS支持的域添加到下面的代码中 */

	ADDDOMAIN(route);						/* 路由支持 */
	ADDDOMAIN(inet);						/* Internet网络域支持 */

	for(dp = domains; dp != NULL; dp = dp->dom_next) {
		if(dp->dom_init)
			(*dp->dom_init)();
		for(pp = dp->dom_proto; pp < dp->dom_proto_last; ++pp) 
			if(pp->pr_init)
				(*pp->pr_init)();
	}
	max_linkhdr = 16;
	max_hdr = max_linkhdr + max_protohdr;
}

/**
 * 描述：根据指定的网络域、socket类型、协议编号查找对应的protocol结构
 * 		 匹配规则：
 * 		 1）网络域、socket类型、协议编号完全匹配
 * 		 2）查找socket类型为SOCK_RAW，并且对应protocol的协议为IPPROTO_IP
 * 返回：找到返回protocol指针，否则返回NULL
 */
struct protocol *profindbyproto(u_int8_t family, \
			u_int16_t socktype, u_int16_t proto)
{
	struct domain *dp;
	struct protocol *pp, *maybe = NULL;

	if(!socktype)				//socket类型非法
		return NULL;

	for(dp = domains; dp != NULL; dp = dp->dom_next) 
		if(dp->dom_family == family) 
			goto found;

	return NULL;

found:
	for(pp = dp->dom_proto; pp < dp->dom_proto_last; ++pp) {
		if(pp->pr_protocol == proto && pp->pr_type == socktype)
			return pp;
		if(SOCK_RAW == socktype && pp->pr_type == socktype \
			&& IPPROTO_IP == pp->pr_protocol && !maybe)
			maybe = pp;
	}

	return maybe;
}

/**
 * 描述：根据指定的网络域、socket类型查找对应的protocol结构
 * 		 匹配规则：
 * 		 1）返回第一个网络域、socket类型匹配上的protocol
 * 返回：找到返回protocol指针，否则返回NULL
 */
struct protocol *profindbytype(u_int8_t family, \
			u_int16_t socktype)
{
	struct domain *dp;
	struct protocol *pp;

	if(!socktype)				//socket类型非法
		return NULL;

	for(dp = domains; dp != NULL; dp = dp->dom_next) 
		if(dp->dom_family == family) 
			goto found;

	return NULL;

found:
	for(pp = dp->dom_proto; pp < dp->dom_proto_last; ++pp)
		if(pp->pr_type == socktype)
			return pp;

	return NULL;
}
