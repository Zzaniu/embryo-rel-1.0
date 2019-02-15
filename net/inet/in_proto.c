
/**
 * Copyright(c) 2017-2-23 Shangwen Wu	
 *
 * Internet网络域包含协议结构定义
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/domain.h>
#include <sys/protocol.h>

#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp_var.h>
#include <netinet/ip_icmp.h>

extern struct domain inetdomain;

/* Internet协议集合，注意，同一类型的协议的init函数只能被调用一次 */
struct protocol inet_protos[] = {
	{0, 0, 0, &inetdomain, ip_init, NULL, ip_output},	
	{SOCK_DGRAM, IPPROTO_UDP, PRF_ATOMIC|PRF_ADDR, &inetdomain, udp_init, udp_usrreq, udp_output, udp_input, NULL},	/* UDP协议 */
	{SOCK_RAW, IPPROTO_ICMP, PRF_ATOMIC|PRF_ADDR, &inetdomain, NULL, rip_usrreq, rip_output, icmp_input, NULL},	/* ICMP协议 */
	{SOCK_RAW, IPPROTO_IP, PRF_ATOMIC|PRF_ADDR, &inetdomain, rip_init, rip_usrreq, rip_output, NULL, NULL},	/* 未匹配成功的默认协议 */
	/* 注意：所有暂时未支持的协议都默认使用下面的原始协议进行路由，因此rip_input必须实现，否则可能引起空指针异常 */
	{SOCK_RAW, IPPROTO_RAW, PRF_ATOMIC|PRF_ADDR, &inetdomain, NULL, rip_usrreq, rip_output, rip_input, NULL},	/* 纯原始协议，不带任何协议头 */
};

/* Internet域定义 */
struct domain inetdomain = 
{
	.dom_family = AF_INET,
	.dom_name = "Internet",
	.dom_proto = inet_protos,
	.dom_proto_last = inet_protos + NR(inet_protos),
	.dom_next = NULL,
	.dom_init = NULL,
	.dom_rtattach = rn_inithead,	//初始化当前domain对应的radix树
	.dom_externalize = NULL,
	.dom_rtoff = 32,		//跳过4个字节，从sockaddr_in的sin_addr字段开始用于key比较
	.dom_maxrtkey = sizeof(struct sockaddr_in),
};

