
/**
 * Copyright(c) 2017-1-17 Shangwen Wu 
 *
 * 设备自动配置实现
 * 
 */
#include <common.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/list.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/autoconf.h>

extern struct cfdata cfdatas[];			//defined in ioconf.c 

extern void panic(char *msg);			//defined in kern_misc.c

struct list_head alldevslist;			//BIOS设备链表

/**
 * 描述：configure之前的准备工作，主要负责初始化各种全局链表 
 *  
 */
void config_init(void)
{
	INIT_LIST_HEAD(&alldevslist);
}

void configure(void)
{
	log(LOG_DEBUG, "begin device config\n");

	if(config_rootfound("mainbus", NULL)) 
		panic("mainbus not found");
		
	log(LOG_DEBUG, "device config done.\n");
}

static void *mapply(struct matchinfo *info, struct cfdata *cf)
{
	void *match = NULL;
	int pri;

	log(LOG_DEBUG, "probe for %s\n", cf->cf_driver->cd_name);

	if(info->indirect)
		match = create_device(cf, info->parent);
	else
		match = cf;

	if(info->fn) {
		pri = (*info->fn)(info->parent, match, info->aux);
	} else {
		if(!cf->cf_attach->ca_match) 
			panic("no match function for device search");	
		pri = (*cf->cf_attach->ca_match)(info->parent, match, info->aux);
	}

	log(LOG_DEBUG, "%s probe pri = %d\n", cf->cf_driver->cd_name, pri);

	if(pri > info->pri)	{						//找到更高优先级匹配项
		if(info->indirect && info->match)
			destroy_device((struct device *)info->match);
		info->match = match;	
		info->pri = pri;
	} else if(info->indirect && match)
		destroy_device((struct device *)match);

	return info->match;
}

/**
 * 查询cfdatas数组，找到符合rootname名字的最优匹配项
 */
static void *config_rootsearch(char *rootname, void *aux, cfmatch_t fn)
{
	extern short roots[];					//defined in ioconf.c
	short *proot;
	void *match = NULL;
	struct matchinfo info;
		
	info.fn = fn;
	info.aux = aux;
	info.pri = 0;
	info.match = NULL;
	info.indirect = 0;
	info.parent = ROOT;

	for(proot = roots; *proot >= 0; ++proot) {
		if(0 == strcmp(rootname, cfdatas[*proot].cf_driver->cd_name))
			match = mapply(&info, &cfdatas[*proot]);
	}

	return match;
}

/**
 * 查询cfdatas数组，找到当前父设备的最优匹配项
 */
static void *config_search(struct device *parent, void *aux, cfmatch_t fn)
{
	void *match = NULL;
	struct cfdata *cf;
	struct matchinfo info;
		
	info.fn = fn;
	info.aux = aux;
	info.pri = 0;
	info.match = NULL;
	info.indirect = (parent && parent->dv_cfdata->cf_driver->cd_indirect);
	info.parent = parent;

	for(cf = cfdatas; cf->cf_attach && cf->cf_driver; ++cf) {
		if(CFSTAT_FOUND == cf->cf_stat)
			continue;
		if(&cfdatas[cf->cf_parent] == parent->dv_cfdata)		//cf为当前parent的子设备
			match = mapply(&info, cf);
	}

	return match;
}

/**
 * 描述：将数字转换成字符串
 * 参数：num，要转换的数字，buf，数字缓冲区，size，缓冲区大小
 * 返回；并返回数字字符串的首地址
 */
static char *number(unsigned short num, char *buf, int size)
{
	char *numstr;	

	if(size < 2 || !buf)
		return NULL;

	numstr = buf + size - 1;
	*numstr = 0;
	do {
		*(--numstr) = (num % 10) + '0';
		num /= 10;
	}
	while(num && numstr > buf);
	
	if(numstr <= buf)							//buf装不下了
		return NULL;

	return numstr;
}

struct device *create_device(struct cfdata *cf, struct device *parent)
{
	struct device *dev;
	struct cfdriver *cd;
	struct cfattach *ca;
	int xsize, nsize;
	char xbuf[10], *xname;
	
	cd = cf->cf_driver;
	ca = cf->cf_attach;
	
	if(ca->ca_devsize < sizeof(struct device)) {
		log(LOG_ERR, "create_device: attach device size too small\n");
		goto failed;
	}

	if(NULL == (dev = (struct device *)kmem_zmalloc(ca->ca_devsize))) {
		log(LOG_ERR, "create_device: out of memory for create device\n");	
		goto failed;
	}
	
	if(CFSTAT_DYNAMIC == cf->cf_stat) 				//动态设备
		dev->dv_unit = cf->cf_unit++;	
	else  											//静态设备
		dev->dv_unit = cf->cf_unit;

	nsize = strlen(cd->cd_name);
	xname = number(dev->dv_unit, xbuf, 10);
	if(!xname || nsize + (xsize = strlen(xname)) > MAX_DEV_NAME) {
		log(LOG_ERR, "device %s-%d name too long\n", cd->cd_name, dev->dv_unit);
		goto failed_free;
	}
	strcpy(dev->dv_name, cd->cd_name);
	strcpy(dev->dv_name + nsize, xname);

	dev->dv_class = cd->cd_class;
	dev->dv_parent = parent;
	dev->dv_cfdata = cf;
	dev->dv_flags = DVF_ACTIVE;

	list_add_tail(&dev->dv_ownnode, &cd->cd_devlist);	//插入到驱动的设备链表
	list_add_tail(&dev->dv_node, &alldevslist);		//插入全局设备管理链表
	
	dev->dv_refcnt = 2;								//以上两个链表引用了该device指针


	return dev;
	
failed_free:
	if(CFSTAT_DYNAMIC == cf->cf_stat) 				//动态设备
		cf->cf_unit--;
	kmem_free(dev);

failed:
	return NULL;
}

void destroy_device(struct device *dev)
{
	struct cfdata *cf;
	struct cfdriver *cd;

	cf = dev->dv_cfdata;
	cd = cf->cf_driver;

	list_del(&alldevslist);
	list_del(&cd->cd_devlist);
	dev->dv_refcnt -= 2;							//减去两个链表的引用
	if(dev->dv_refcnt)
		panic("destroy device which is referenced");

	if(CFSTAT_DYNAMIC == cf->cf_stat) 				//动态设备
		cf->cf_unit--;
	kmem_free(dev);
}

int config_attach(struct device *parent, void *match, void *aux)
{
	int err;
	struct device *dev;
	struct cfdata *cf;
	struct cfattach *ca;

	if(parent && parent->dv_cfdata->cf_driver->cd_indirect) {
		dev = (struct device *)match;
		cf = dev->dv_cfdata;
	} else {
		cf = (struct cfdata *)match;
		dev = create_device(cf, parent);
	}
	if(!dev || !cf) {
		log(LOG_ERR, "%s: match is null\n", __func__);
		return -1;
	}

	ca = cf->cf_attach;

	if(CFSTAT_STATIC == cf->cf_stat) 				//动态设备
		cf->cf_stat = CFSTAT_FOUND;	

	if(ROOT == parent) 
		log(LOG_INFO, "%s (root)\n", dev->dv_name);
	else 
		log(LOG_INFO, "%s at %s\n", dev->dv_name, parent->dv_name);

	//执行具体设备的attach函数
	if(err = (*ca->ca_attach)(parent, dev, aux)) 	//设备attach/dettach函数
		goto failed;	

	return 0;

failed:
	log(LOG_ERR, "%s: device attach failed, err %d\n", __func__, err);
	destroy_device(dev);

	return -1;
}

/**
 * 描述：遍历整个设备数，过程分为两部分，sercah函数负责查找匹配项，
 * 		 attach将执行具体的设备attach函数，并引起递归操作
 */
int config_rootfound(char *rootname, void *aux)
{
	void *match;

	if((match = config_rootsearch(rootname, aux, NULL)) != NULL)
		return config_attach(ROOT, match, aux);
	
	log(LOG_WARNING, "config_rootfound: root device %s not configured\n", rootname);

	return -1;
}

/**
 * 描述：类似于config_rootfound，用于root的直接子设备之外的其他间接设备，过程分为两部分，
 * 		 sercah函数负责查找匹配项，attach将执行具体的设备attach函数，并引起递归操作
 */
int config_found(struct device *parent, void *aux, cfmatch_t fn)
{
	void *match;

	if((match = config_search(parent, aux, fn)) != NULL)
		return config_attach(parent, match, aux);
	
	log(LOG_DEBUG, "config_found: %s's subdevice probe failed\n", parent->dv_name);

	return -1;
}

/**
 * 描述：作用类似于config_found，用于root的直接子设备之外的其他间接设备，但是不同于其他
 * 		 found函数直接调用mapply函数寻找当前设备的最优匹配子设备，该函数将寻找并attach
 * 		 当前设备下的所有子设备，而非仅attach最优匹配的子设备，至于具体的扫描规则将回调
 * 		 当前设备自己的扫描函数，扫描函数将初始化一些基本的attach配置参数
 */
void config_scan(struct device *parent, cfscan_t fn)
{
	void *match = NULL;
	struct cfdata *cf;
	unsigned char indirect;
		
	indirect = (parent && parent->dv_cfdata->cf_driver->cd_indirect);

	for(cf = cfdatas; cf->cf_attach && cf->cf_driver; ++cf) {
		if(CFSTAT_FOUND == cf->cf_stat)
			continue;
		if(&cfdatas[cf->cf_parent] == parent->dv_cfdata) {		//cf为当前parent的子设备
			match = indirect ? (void *)create_device(cf, parent) : (void *)cf;
			if(!match)											//设备创建失败
				continue;
			(*fn)(parent, match);
		}
	}
}

