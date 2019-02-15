
/**
 * Copyright(c) 2017-8-21 Shangwen Wu	
 *
 * linux ifconfig实现 
 * 
 */
#include <string.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <command.h>
#include <unistd.h>
#include <fs/termio.h>
#include <sys/list.h>
#include <sys/types.h>					
#include <sys/socket.h>					//for socket API
#include <sys/sockio.h>	
#include <sys/endian.h>		

#include <net/if.h>
#include <net/if_type.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>

static void setsockin(struct sockaddr_in *sin, sa_family_t family, in_addr_t addr)
{
	bzero((caddr_t)sin, sizeof(*sin));
	sin->sin_len = sizeof(*sin);
	sin->sin_family = family;
	sin->sin_addr.s_addr = addr;
}

/**
 * 描述：显示系统网络接口信息 
 * 参数：ifr中要包含指定网络接口的设备名称，注意：ifr参数的实际存储空间应当为in_ifaliasreq的大小，bad code
 * 		 needup，表示是否仅显示UP的网络接口
 */
static int display_ifinfo_up(int sckfd, struct ifreq *ifr, int needup)
{
	int err = -1;
	ushort flags;
	struct if_data *ifd;
	struct sockaddr_in addr, mask, broadaddr;
	uint8_t hwaddr[6] = {0};
	char hwaddr_str[32];

	//下面的接口名称要和其宏定义值相对应
	static char *encapnames[] = {"Unkown", "Local Loopback", "Ethernet"};

	if(ioctl(sckfd, SIOCGIFFLAGS, ifr))
		return -1;	
	flags = ifr->ifr_flags;

	if(needup && !(flags & IFF_UP))
		return 0;

	satosin(&ifr->ifr_addr)->sin_addr.s_addr = INADDR_ANY;
	if(!ioctl(sckfd, SIOCGIFADDR, ifr)) {
		addr = *satosin(&ifr->ifr_addr);
		if(addr.sin_addr.s_addr != INADDR_ANY && addr.sin_addr.s_addr != INADDR_NONE) {
			satosin(&ifr->ifr_addr)->sin_addr.s_addr = INADDR_ANY;
			if(ioctl(sckfd, SIOCGIFNETMASK, ifr))
				return -1;	
			mask = *satosin(&ifr->ifr_addr);
			if(flags & IFF_BROADCAST) {
				satosin(&ifr->ifr_broadaddr)->sin_addr.s_addr = INADDR_ANY;
				if(ioctl(sckfd, SIOCGIFBRDADDR, ifr))
					return -1;	
				broadaddr = *satosin(&ifr->ifr_broadaddr);
			}
		}
	} else if(errno == EADDRNOTAVAIL) {
		addr.sin_addr.s_addr = INADDR_NONE; 
	} else {
		return -1;
	}

	if(NULL == (ifd = (struct if_data *)malloc(sizeof(struct if_data)))) {
		errno = ENOBUFS;
		return -1;
	}
	ifr->ifr_data = (caddr_t)ifd; 
	
	if(ioctl(sckfd, SIOCGIFDATA, ifr))
		goto failed;

	if(IFT_ETHER == ifd->ifd_type) {
		ifr->ifr_data = (caddr_t)hwaddr; 
		if(ioctl(sckfd, SIOCGETETHERADDR, ifr))
			goto failed;
	}

	/* 显示接口信息 */
	printf("%-10s", ifr->ifr_name);
	printf("Link encap:%s  ", encapnames[ifd->ifd_type]);
	if(IFT_ETHER == ifd->ifd_type) {
		sprintf(hwaddr_str, "HWaddr %x:%x:%x:%x:%x:%x", \
			hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
		printf(hwaddr_str);
	}

	if(addr.sin_addr.s_addr != INADDR_ANY && addr.sin_addr.s_addr != INADDR_NONE) {
		printf("\n%*s inet addr:%s  ", 9, " ", inet_ntoa(addr.sin_addr));
		if(flags & IFF_BROADCAST)
			printf("Bcast:%s  ", inet_ntoa(broadaddr.sin_addr));
		printf("Mask:%s", inet_ntoa(mask.sin_addr));
	}

	printf("\n%*s ", 9, " ");
	printf("%s", flags & IFF_UP ? "UP " : "");
	printf("%s", flags & IFF_LOOPBACK ? "LOOPBACK " : "");
	printf("%s", flags & IFF_POINT2POINT ? "POINTTOPOINT " : "");
	printf("%s", flags & IFF_RUNNING ? "RUNNING " : "");
	printf("%s", flags & IFF_MULTICAST ? "MULTICAST " : "");
	printf("%s", flags & IFF_BROADCAST ? "BROADCAST " : "");
	printf(" MTU:%lu ", ifd->ifd_mtu);
	printf("Metric:%lu\n", ifd->ifd_metric);
	printf("%*s Rx packets:%lu\n", 9, " ", ifd->ifd_ipackets);
	printf("%*s Tx packets:%lu\n", 9, " ", ifd->ifd_opackets);
	printf("%*s Rx bytes:%lu(%lu.0 b)  ", 9, " ", ifd->ifd_ibytes, ifd->ifd_ibytes << 3);
	printf("Tx bytes:%lu(%lu.0 b)\n", ifd->ifd_obytes, ifd->ifd_obytes << 3);

	err = 0;

failed:
	free(ifd);

	return err;
}

/**
 * 描述：显示当前系统下所有网络接口信息
 * 参数：needup，为1时，仅查询对应状态为UP的网络接口
 */ 
static int display_ifinfo_all(int sckfd, int needup)
{
	int err = -1;
	ulong len;
	struct ifconf ifc;
	struct in_ifaliasreq data;
	struct ifreq *ifr, *ifr_d = (struct ifreq *)&data;
	struct sockaddr *sa;
	caddr_t old_name = "";

	/* 获取缓冲区大小 */
	ifc.ifc_len = 0;
	if(ioctl(sckfd, SIOCGIFCONF, &ifc)) {
		fprintf(stderr, "error fetching system interface information: %s\n", strerror(errno));
		return -1;
	}
	len = ifc.ifc_len;
	
	if(NULL == (ifr = (struct ifreq *)malloc(len))) {
		fprintf(stderr, "error fetching system interface information: Out of memory\n");
		return -1;
	}
	ifc.ifc_req = ifr;
	if(ioctl(sckfd, SIOCGIFCONF, &ifc)) {
		fprintf(stderr, "error fetching system interface information: %s\n", strerror(errno));
		goto failed;
	}
	
	/* 遍历所有网络接口 */	
	while(len > sizeof(struct ifreq)) {
		sa = &ifr->ifr_addr;
		if(sa->sa_len > sizeof(struct sockaddr)) 
			if(len < sizeof(struct ifreq) + (sa->sa_len - sizeof(struct sockaddr)))
				break;
		//重复的接口不再显示信息
		if(strncmp(ifr->ifr_name, old_name, IFNAMESZ)) {
			bcopy(ifr->ifr_name, ifr_d->ifr_name, IFNAMESZ);
			if(display_ifinfo_up(sckfd, ifr_d, needup)) {
				fprintf(stderr, "%s: error fetching interface information: %s\n", ifr->ifr_name, strerror(errno));
				goto failed;
			}
			old_name = ifr->ifr_name;
		}
		if(sa->sa_len > sizeof(struct sockaddr)) {
			len -= sizeof(struct ifreq) + (sa->sa_len - sizeof(struct sockaddr));
			ifr = (struct ifreq *)((caddr_t)(ifr + 1) + (sa->sa_len - sizeof(struct sockaddr)));
		} else {
			len -= sizeof(struct ifreq);
			++ifr;
		}
	}

	err = 0;

failed:
	free(ifc.ifc_req);

	return err;
}

/**
 * 描述：ifconfig命令执行函数
 * 参数：argc表示参数个数，argv表示参数数组
 * 返回：函数执行结果，返回0表成功
 */
static int cmd_ifconfig(int argc, const char *argv[])
{
	int sckfd, i;
	struct in_ifaliasreq data, *inifar = &data;
	struct ifreq *ifr = (struct ifreq *)&data;
	char *opts[] = {"up", "down"};

	if(-1 == (sckfd = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("socket");
		return -1;
	}
	
#define exit_whitherr(msg) 	{perror(msg); close(sckfd); return -1;} 

	if(1 == argc) {							//显示所有状态为UP的接口信息
		if(display_ifinfo_all(sckfd, 1)) {
			close(sckfd);
			return -1;
		}
	} else if(2 == argc){
		if(!strcmp(argv[1], "-a")) {		//显示所有接口信息		
			if(display_ifinfo_all(sckfd, 0)) {
				close(sckfd);
				return -1;
			}
		} else {							//显示指定接口信息
			strncpy(ifr->ifr_name, argv[1], min(IFNAMESZ - 1, strlen(argv[1])));
			ifr->ifr_name[min(IFNAMESZ - 1, strlen(argv[1]))] = 0;
			if(display_ifinfo_up(sckfd, ifr, 0)) {
				fprintf(stderr, "%s: error fetching interface information: %s\n", ifr->ifr_name, strerror(errno));
				close(sckfd);
				return -1;
			}
		}
	} else {
		strncpy(ifr->ifr_name, argv[1], min(IFNAMESZ - 1, strlen(argv[1])));
		ifr->ifr_name[min(IFNAMESZ - 1, strlen(argv[1]))] = 0;
		for(i = 0; i < NR(opts); ++i)
			if(0 == strcmp(argv[2], opts[i]))
				break;
		switch(i) {
			case 0:					//up
				if(ioctl(sckfd, SIOCGIFFLAGS, ifr))
					exit_whitherr("up");
				if(ifr->ifr_flags & IFF_UP)
					break;			//避免重复打开
				ifr->ifr_flags |= IFF_UP;
				if(ioctl(sckfd, SIOCSIFFLAGS, ifr))
					exit_whitherr("up");
				break;
			case 1:					//down
				if(ioctl(sckfd, SIOCGIFFLAGS, ifr))
					exit_whitherr("down");
				if(!(ifr->ifr_flags & IFF_UP))
					break;			//避免重复关闭
				ifr->ifr_flags &= ~IFF_UP;
				if(ioctl(sckfd, SIOCSIFFLAGS, ifr))
					exit_whitherr("down");
				break;
			default:				//set if address
				setsockin(&inifar->iifar_addr, AF_INET, inet_addr(argv[2]));
				if(ioctl(sckfd, SIOCSIFADDR, inifar))
					exit_whitherr("setifaddr");
				if(argc >= 4) {
					setsockin(&inifar->iifar_mask, AF_INET, inet_addr(argv[3]));
					if(ioctl(sckfd, SIOCSIFNETMASK, inifar))
						exit_whitherr("setifmask");
				}
		}
	}

	close(sckfd);

	return 0;
}

/* help命令的选项描述 */
static struct optdesc ifconfig_opts[] = {
	{
		.name = "-a", 
		.desc = "displays the status of all interfaces",
	},
	{
		.name = "interfaces", 
		.desc = "the name of network interfaces, e.g. eth0",
	},
	{
		.name = "options", 
		.desc = "options of configuration",
	},
	{
		.name = "address", 
		.desc = "interface address, e.g. IP address",
	},
	{},
};

/* 一组相关命令的定义，数组第一个元素为组名 */
static struct cmd cmds[] = {
	{{"Network"}},			//表组名
	{
		{"ifconfig"}, 			
		"configure a network interface", 
		"[-a]|[interface]|interface options|address ...",
		ifconfig_opts, 
		cmd_ifconfig,
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
