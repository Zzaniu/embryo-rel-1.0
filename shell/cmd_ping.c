/**
 * Copyright(c) 2017-2-20 Shangwen Wu	
 *
 * ping命令实现 
 * 
 */

#include <common.h>
#include <command.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <fs/termio.h>
#include <sys/types.h>					
#include <sys/endian.h>					
//#include <sys/time.h>					//for gettimeofday API
#include <sys/socket.h>					//for socket API
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>					//for inet_addr API

//ICMP包最大长度为以太网最大长度（1500B）减去IP协议头最大长度（60B）
#define MAXPACKSIZE		(1500-60)
//ICMP包最小长度为以太网最小长度（46B）减去IP协议头最小长度（20B）
#define MINPACKSIZE		(64-14-4-20)
//ICMP默认数据长度为56字节，数据包括一个时间戳以及剩余填充字节
#define DATALEN			(64-ICMP_MINLEN)

/* 最大IP报文长度 */
#define MAXIPLEN		60
/* 最大ICMP报文长度 */	
#define MAXICMPLEN		76

static int sckfd;						//socket文件描述副
static uchar outpack[MAXPACKSIZE];		//输出缓冲区
static uchar inpack[MAXPACKSIZE];		//输入缓冲区
static struct sockaddr whereto;			//发送目标地址
static int datalen;						//发送数据长度
static int timing;						//是否发送时间戳
static unsigned short ntransmitted;		//已发送个数

static void usage(void)
{
	fprintf(stderr, "Usage: ping [-c count] [-s packetsize] Host\n");
}

u16 getchksum(u16 *packet, int len)
{
	u32 sum = 0;
	u16 *pos = packet;
	while(len - 2 >= 0)
	{		
		sum += *pos++;
		len -= 2;
	}
	if(1 == len)
		sum += *((u8 *)pos);
	sum = (sum >> 16) + (sum & 0xffff);
	while(sum >> 16)
		sum += (sum >> 16);
	return ~sum;
}

static int pinger(void)
{
	int ret, outlen = datalen + ICMP_MINLEN;
	struct icmp *icmph = (struct icmp *)outpack;
	//struct timeval *tv = NULL;
	
	icmph->icmp_type = ICMP_ECHO;
	icmph->icmp_code = 0;
	icmph->icmp_cksum = 0;
	//icmph->icmp_id = htons(getpid());
	icmph->icmp_id = htons(0x1234);
	icmph->icmp_seq = htons(ntransmitted++);
	
	//tv = (struct timeval *)icmph->icmp_data;
	//if(-1 == gettimeofday(tv, NULL))
	//{
	//	perror("gettimeofday");
	//	return -1;
	//}
	icmph->icmp_cksum = getchksum((u16 *)outpack, outlen);	

	if(-1 == (ret = sendto(sckfd, outpack, outlen, 0, &whereto, sizeof(struct sockaddr)))) 
		perror("sendto");	

	return ret;
}

/**
 * 描述：do_ping函数
 */
int do_ping(unsigned int argc, const char *argv[])
{
	int times = 1, i, inlen, ret;
	socklen_t fromlen;
	const char *target = NULL;
	uchar *datap = NULL;
	struct sockaddr_in *to;
	struct sockaddr_in wherefrom = {0};
	
	datalen = DATALEN;
	timing = 0;
	ntransmitted = 0;

	if(argc < 2) {
		usage();
		return -1;
	}

	//填充发送数据包
	//if(datalen >= sizeof(struct timeval))
	//	timing = 1;	
	//if(timing)
	//	datap = &outpack[ICMP_MINLEN + sizeof(struct timeval)];
	//else
		datap = &outpack[ICMP_MINLEN];
	for(i = 0; datap < outpack + datalen + ICMP_MINLEN; ++i)
		*datap++ = i;

	//初始化发送的目标地址
	target = argv[1];
	to = (struct sockaddr_in *)&whereto;
	bzero((char *)&whereto, sizeof(struct sockaddr));
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(target);

	if(-1 == (sckfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))) {
		perror("socket");
		return -1;
	}

	while(times--) {
		if(-1 == pinger()) {
			fprintf(stderr, "send packet failed\n");
			break;
		}
		printf("send one packet to %s\n", target);

		//wsw deubg
		//continue;

		inlen = datalen + MAXIPLEN + MAXICMPLEN;
		fromlen = sizeof(wherefrom);
		if((ret = recvfrom(sckfd, inpack, inlen, 0, (struct sockaddr *)&wherefrom, &fromlen)) != -1) { 
			printf("recv one packet from %s\n", inet_ntoa(wherefrom.sin_addr));
			printf("recv buf[%d]: ", ret);
			for(i = 0; i < ret; ++i)
				printf("0x%x ", inpack[i]);
			printf("\n");
		} else
			perror("recvfrom");
	}

	close(sckfd);
	
	return 0;
}

/**
 * 描述：执行hello测试命令
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_ping(int argc, const char *argv[])
{
	return do_ping(argc, argv);
}

static struct optdesc ping_opts[] = {
	{
		.name = "host", 
		.desc = "destination host",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Network"}},			//表组名
	{
		{"ping"}, 			
		"ping remote host", 
		"host",
		ping_opts, 
		cmd_ping,
		0,
		MAX_CMD_ARG_NUM - 1,
		0, 
	},
	{},						//最后一个强制要求为空
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) init_cmds(void)
{
	add_cmds(cmds, 0);
}
