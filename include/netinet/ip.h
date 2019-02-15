
/**
 * Copyright(c) 2017-4-30 Shangwen Wu	
 *
 * Ip协议实现
 * 
 */

#ifndef __NETINET_IP_H__
#define __NETINET_IP_H__

#define IPVERSION	4			//IPV4

struct in_addr;//defined in in.h

/* IP协议头定义 */
struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int8_t ip_hlen:4; 		/* IP头长度 */
	u_int8_t ip_ver:4; 			/* IP版本号，IPV4 */
#elif BYTE_ORDER == BIG_ENDIAN
	u_int8_t ip_ver:4; 
	u_int8_t ip_hlen:4; 
#endif
	u_int8_t ip_tos;			/* 服务质量 */
	u_int16_t ip_len;			/* IP报文总长 */
	u_int16_t ip_id;			/* IP报文序列号 */
	u_int16_t ip_off;			/* 分片偏移以及标志位 */
#define IP_OFFMASK		0x1fff	/* IP分片偏移掩码 */
#define IP_RESERVEFRAG	0x8000	/* 预留bit */
#define IP_DONTFRAG		0x4000	/* 标志是否是禁止IP分片 */
#define IP_MOREFRAG		0x2000	/* 标志是否是IP分片的最后一片 */
	u_int8_t ip_ttl;			/* TTL生存跳数 */
	u_int8_t ip_pro;			/* IP之上的协议类型 */
	u_int16_t ip_sum;			/* IP头校验和 */
	struct in_addr ip_src, ip_dst;	/* 源地址与目的地址 */
};

/* 用于方便计算UDP & TCP伪头部校验和的特殊IP头组织形式，该结构体的大小必须为一个不带选项数据的IP头大小 */
struct ipovly {
	uint8_t ipv_resv[9];		/* 在进行伪头部计算时，该字段必须为全0，从而不会影响伪头部校验和结果 */
	uint8_t ipv_pro;			/* 下面几个字段属于伪头部，但是真正的伪头部字段顺序与下面相反 */
	uint16_t ipv_len;			/* 之所以颠倒顺序，是因为方便后面组装IP头，这里颠倒顺序并不会影响校验和的计算 */
	struct in_addr ipv_src, ipv_dst;	/* 源地址与目的地址 */
};

#define IP_MAXPACKET		65535			/* 一个IP报文允许的最大长度 */

#define IPMAXTTL			255				/* ping命令用到的默认TTL */
#define IPDEFTTL			64				/* udp协议使用的TTL */

/* IP选项公共字段偏移 */
#define MAXIPOPTLEN		40		/* 选项最大长度 */


#define IPOPT_CODE		0		/* IP选项码 */
#define IPOPT_LEN		1		/* IP选项长度 */
#define IPOPT_PTR		2		/* 当前选项数据相对于选项起始位置偏移,偏移以1开始 */
#define IPOPT_MINOFF	4		/* ipo_ptr的最小值 */


/* IP选项码 */
#define IPOPT_EOL		0
#define IPOPT_NOP		1
#define IPOPT_RR		7
#define IPOPT_TS		68
#define IPOPT_LSRR		131
#define IPOPT_SSRR		137

/* IP选项时间戳定义 */
struct ip_timestamp {
	uint8_t ipt_code;
	uint8_t ipt_len;
	uint8_t ipt_ptr;
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t ipt_flg:4;			/* 标志位 */
	uint8_t ipt_oflw:4;			/* 溢出位 */
#elif BYTE_ORDER == BIG_ENDIAN
	uint8_t ipt_oflw:4;			/* 溢出位 */
	uint8_t ipt_flg:4;			/* 标志位 */
#endif
	union ipt_timestamp {
		n_time ipt_time[1];
		struct ipt_ta {			/* 当指定IP时，将保存IP地址 */
			struct in_addr ipt_addr;
			n_time ipt_time;
		}ipt_ta[1];
	}ipt_timestamp;
};

#define IPOPT_TS_TSONLY		0	/* 仅保存时间戳 */
#define IPOPT_TS_TSANDADDR	1	/* 保存时间戳以及自己的IP地址 */
#define IPOPT_TS_PRESPEC	3	/* 源指定的IP路由保存时间戳 */

#endif //__NETINET_IP_H__
