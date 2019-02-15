
/**
 * Copyright(c) 2017-1-17 Shangwen Wu	
 *
 * 设备配置信息，此文件将动态生成
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <sys/device.h>

extern struct cfattach mainbus_ca;		//defined in mainbus.c 
extern struct cfdriver mainbus_cd;
extern struct cfattach localbus_ca;		//defined in localbus.c 
extern struct cfdriver localbus_cd;
extern struct cfattach stmmac_ca;		//defined in stmmac_main.c 
extern struct cfdriver stmmac_cd;
extern struct cfattach ahci_plt_ca;		//defined in ahci_platform.c 
extern struct cfdriver ahci_plt_cd;
extern struct cfattach ahci_disk_ca;	//defined in ahci_disk.c 
extern struct cfdriver ahci_disk_cd;

extern void loopattach(int n);

/* 所有外设的IO地址表 */
uint32_t iob[] = {
	-1, 0xbfe10000, 0xbfe18000, 0xbfe30000, 
};

/* 设备信息配置表，此表应当通过配置文件生成 */
struct cfdata cfdatas[] = {
	/* attachment     driver    state    parent    unit    iobase */
	/* 0: mainbus0 at root */
	{&mainbus_ca, &mainbus_cd, CFSTAT_STATIC, -1, 0, iob},
	/* 1: localbus0 at mainbus0 */
	{&localbus_ca, &localbus_cd, CFSTAT_STATIC, 0, 0, iob},
	/* 2: stmmac0 at localbus0 */
	{&stmmac_ca, &stmmac_cd, CFSTAT_STATIC, 1, 0, iob + 1},
	/* 3: stmmac1 at localbus0 */
	{&stmmac_ca, &stmmac_cd, CFSTAT_STATIC, 1, 1, iob + 2},
	/* 4: ahci_plt0 at localbus0 */
	{&ahci_plt_ca, &ahci_plt_cd, CFSTAT_STATIC, 1, 0, iob + 3},
	/* 5: ahci_disk* at ahci_plt0 */
	{&ahci_disk_ca, &ahci_disk_cd, CFSTAT_DYNAMIC, 4, 0, iob},
	{0},										//driver && attachment = 0 表示数组元素结束
};

/* 当前系统下所有的root设备，每个元素为cfdatas数组的索引 */
short roots[] = {
	0,											//mainbus0
	-1,											//-1表示数组元素结束
};

/* 虚拟设备数组 */
struct pdevinfo pdevinfo[] = {
	{loopattach, 1},
	{NULL, 0},
};

