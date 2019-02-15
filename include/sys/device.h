
/**
 * Copyright(c) 2017-1-17 Shangwen Wu	
 *
 * embryo驱动框架相关头文件
 * 
 */

#ifndef __SYS_DEVICE_H__
#define __SYS_DEVICE_H__

#include <sys/list.h>						//for list_head

#define CFSTAT_STATIC  	0					//表示片上设备，这些设备一般是芯片固有的，依赖具体使用的芯片
#define CFSTAT_FOUND	1					//表示设备已经找到，用于静态设备
#define CFSTAT_DYNAMIC	2					//表示不确定的动态设备，这些一般设备挂接在某些总线上，比如pci，usb

#define ROOT 			((struct device *)0)	//根设备

enum devclass {
	DEV_DULL,								//一般设备
	DEV_TTY,								//TTY设备
	DEV_NET,								//网络设备
	DEV_DISK,								//磁盘设备
};

struct device;

typedef int (*cfmatch_t)(struct device *, void *, void *);		//在设备探测时，返回一个匹配优先级

/* 方便匹配用的信息结构 */
struct matchinfo {
	cfmatch_t fn;							//优先使用serch函数自定义的cfmatch函数返回的优先级
	struct device *parent;					//当前匹配项的父设备
	void *match;							//暂存已经匹配的结果信息
	void *aux;								//sercha函数传递的aux辅助信息
	int	pri; 								//上一次匹配成功的最高优先级
	unsigned char indirect; 				//当前设备是否为间接子设备
};

/* 设备驱动信息结构 */
struct cfdriver {
	struct list_head cd_devlist;			//设备链表表头，用于管理其下的设备队列
	char *cd_name;							//驱动名称
	enum devclass cd_class;					//设备类型
	unsigned char cd_indirect;				//间接子设备	
};

/* 描述一个设备的匹配方式以及attach相关的信息结构 */
struct cfattach {
	size_t ca_devsize;						//device结构+私有数据长度之和
	cfmatch_t ca_match;						//根据设备相关的匹配函数
	int (*ca_attach)(struct device *, struct device *, void *);	//设备attach/detach函数
	void (*ca_dttach)(struct device *, struct device *, void *);
};

/* 描述一个设备基本配置信息数据结构，自动生成工具将生成一个由该结构体组成的数组 */
struct cfdata {
	struct cfattach	*cf_attach;
	struct cfdriver *cf_driver;
	short cf_stat;							//描述设备的状态和类型
	short cf_parent;						//指向父设备在cfdatas数组中的索引
	unsigned short cf_unit;					//当cf_stat类型为dynamic设备时，表示探测到的设备最大设备号，也可用来表示当前cfdata共有多少个对应的设备
	uint32_t *cf_iobase;					//指向设备IO地址的指针
};

#define MAX_DEV_NAME		16

#define DVF_DISABLE			0
#define DVF_ACTIVE			1
#define DVF_INACTIVE		2

/* 设备信息结构体 */
struct device {
	struct cfdata *dv_cfdata;
	struct device *dv_parent;				//指向父设备
	struct list_head dv_node;				//设备链表节点，用于设备统一管理
	struct list_head dv_ownnode;			//所属设备驱动的链表节点
	char dv_name[MAX_DEV_NAME];				//设备名称，“驱动名称+设备号”
	enum devclass dv_class;					//设备类型
	int dv_flags;							//设备状态标识
	unsigned short dv_unit;					//设备编号，该设备号是针对同一cfdata类型的设备而言的
	unsigned short dv_refcnt;				//引用计数
};

enum bustype {
	BUS_MAIN, 								//mainbus
	BUS_LOCAL,								//localbus
	BUS_PCIBR,								//pci bridge
	BUS_ISABR,								//isa bridge
};

struct confargs;

/* 总线相关的device私有数据定义 */
struct bushook {
	struct device *bh_dev;					//总线的设备指针
	enum bustype bh_type;					//总线设备类型
	int (*bh_matchname)(struct confargs *, const char *);	//总线名称匹配规则函数
};

/* 总线设备配置参数 */
struct confargs {
	char *ca_name;							//设备名称，这个名称将和cfdriver中的cd_name相比较
	struct bushook *ca_bus;					//总线设备私有数据指针
	uint32_t ca_iobase;						//设备IO地址
};

/* 虚拟设备信息描述 */
struct pdevinfo {
	void (*pdev_attach)(int);				//虚拟设备attach函数
	int pdev_cnt;							//虚拟设备个数
};

typedef void (*cfscan_t)(struct device *, void *);							//设备相关的扫描函数

extern int config_rootfound(char *rootname, void *aux);								//defined in autoconf.c
extern int config_found(struct device *parent, void *aux, cfmatch_t fn);			//defined in autoconf.c
extern void config_scan(struct device *parent, cfscan_t fn);						//defined in autoconf.c
extern int config_attach(struct device *parent, void *match, void *aux);			//defined in autoconf.c
extern struct device *create_device(struct cfdata *cf, struct device *parent);		//defined in autoconf.c
extern void destroy_device(struct device *dev);										//defined in autoconf.c

#endif //__SYS_DEVICE_H__

