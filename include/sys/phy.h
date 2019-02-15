
/**
 * Copyright(c) 2018-4-22 Shangwen Wu	
 *
 * PHY相关定义
 * 
 */

#ifndef __SYS_PHY_H__
#define __SYS_PHY_H__

struct mii_bus;
struct list_head;
struct phy_driver;

/* PHY抽象描述 */
struct phy_device {
	uint32_t phy_id;						//PHY的厂商ID+设备ID，用于匹配PHY驱动
	uint32_t phy_addr;						//PHY地址
	struct phy_driver *driver;				//对应的PHY驱动
	struct mii_bus *bus;					//当前PHY挂接的miibus
	void *attachdev;						//指向网口驱动的设备描述结构
	/* PHY的链路状态信息 */
	int speed, duplex, pause, asyn_pause, link;
	uint32_t supported, advertising, autoneg;
};

/* PHY驱动 */
struct phy_driver {
	uint32_t phy_id;
	uint32_t phy_id_mask;
	char *name;
	uint32_t features;
	struct list_head list;
	int (*phy_match)(struct phy_device *, struct phy_driver *);	//用于phy_device与phy_driver之间的匹配关系，该函数反问一个匹配优先级，值越大匹配优先级越高
	int (*config_init)(struct phy_device *);//PHY配置初始化
	int (*read_status)(struct phy_device *);//获取当前PHY工作状态
};

static inline int phy_read(struct phy_device *dev, uint8_t reg, uint16_t *val)
{
	return (*dev->bus->mii_read)(dev->bus, (uint8_t)dev->phy_addr, reg, val);
}

static inline int phy_write(struct phy_device *dev, uint8_t reg, uint16_t val)
{
	return (*dev->bus->mii_write)(dev->bus, (uint8_t)dev->phy_addr, reg, val);
}

extern int phy_driver_register(struct phy_driver *driver);//defined in phy_device.c
extern int get_phy_id(struct mii_bus *, uint8_t , uint32_t *);
extern struct phy_device *create_phy_device(struct mii_bus *, uint8_t, uint32_t);
extern void free_phy_device(struct phy_device *dev);
extern int phy_attach_driver(struct phy_device *dev);
extern struct phy_device *phy_connect(void *, struct mii_bus *, uint8_t);
extern void phy_disconnect(void *, struct mii_bus *, uint8_t);

#endif //__SYS_PHY_H__
