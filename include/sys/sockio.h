

/**
 * Copyright(c) 2017-8-21 Shangwen Wu	
 *
 * socket ioctl相关定义 
 * 
 */

#ifndef __SYS_SOCKIO_H__
#define __SYS_SOCKIO_H__

#include <sys/ioctl.h>

struct in_ifaliasreq;
struct ifreq;

#define SIOCSIFFLAGS	_IOW('i', 0, struct ifreq)	//设置接口标志位
#define SIOCGIFFLAGS	_IOWR('i', 1, struct ifreq)	//获取接口标志位
#define SIOCSIFMETRIC	_IOW('i', 2, struct ifreq)	//设置接口TTL
#define SIOCGIFMETRIC	_IOWR('i', 3, struct ifreq)	//获取接口TTL
#define SIOCGIFCONF		_IOWR('i', 4, struct ifreq)	//获取系统接口信息
#define SIOCGIFDATA		_IOWR('i', 5, struct ifreq)	//获取系统接口信息数据

#define SIOCSIFADDR		_IOW('i', 6, struct ifreq)	//设置接口地址
#define SIOCGIFADDR		_IOWR('i', 7, struct ifreq)	//获取接口地址
#define SIOCSIFDSTADDR	_IOW('i', 8, struct ifreq)	//设置p2p接口地址
#define SIOCGIFDSTADDR	_IOWR('i', 9, struct ifreq)	//获取p2p接口地址
#define SIOCSIFBRDADDR	_IOW('i', 10, struct ifreq)	//设置接口广播地址
#define SIOCGIFBRDADDR	_IOWR('i', 11, struct ifreq)	//获取接口广播地址
#define SIOCSIFNETMASK	_IOW('i', 12, struct ifreq)	//设置网络掩码
#define SIOCGIFNETMASK	_IOWR('i', 13, struct ifreq)	//获取网络掩码
#define SIOCAIFADDR		_IOW('i', 14, struct ifreq)	//添加接口地址
#define SIOCDIFADDR		_IOW('i', 15, struct ifaliasreq)	//删除接口地址

#define SIOCGETETHERADDR	_IOWR('i', 16, struct ifreq)	//获取以太网地址

#endif //__SYS_SOCKIO_H__

