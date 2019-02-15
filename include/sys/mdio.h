
/**
 * Copyright(c) 2018-4-9 Shangwen Wu	
 *
 * mdio PHY访问相关定义
 * 
 */

#ifndef __SYS_MDIO_H__
#define __SYS_MDIO_H__

#define MAX_NETDEV_NAME		16		//同device.h的MAX_DEV_NAME
#define PHY_MAX		32		//最大PHY地址+1
#define PHY_MASK	0x1f	//最大PHY地址31

struct mii_bus;
struct list_head;
struct phy_device;

struct mii_bus {
	char mii_name[MAX_NETDEV_NAME];	//用于匹配mii总线的网卡设备名
	int mii_id;						//总线ID号，该字段与网卡的dv_unit号对应
	struct list_head mii_list;
	struct phy_device *phy_map[PHY_MAX];	//用于保存当前miibus总线下扫描到的所有PHY设备
	int (*mii_read)(struct mii_bus *, uint8_t, uint8_t, uint16_t *);
	int (*mii_write)(struct mii_bus *, uint8_t, uint8_t, uint16_t);
	void *mii_priv;					//MII总线私有数据
};

extern int mdio_write(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t val); //defined in mdio.c
extern int mdio_read(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t *val); //defined in mdio.c
extern struct mii_bus *mdio_match(const char *devname);	//defined in mdio.c
extern int mdio_register(struct mii_bus *bus);//defined in mdio.c
extern int mdio_unregister(struct mii_bus *bus);//defined in mdio.c
extern int mdio_scan(struct mii_bus *bus, uint8_t phyaddr);//defined in mdio.c

#endif //__SYS_MDIO_H__
