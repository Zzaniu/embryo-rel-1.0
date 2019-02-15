
/**
 * Copyright(c) 2018-4-22 Shangwen Wu	
 *
 * PHY抽象层以及通用PHY驱动相关函数
 * 
 */
#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/err.h>
#include <sys/list.h>
#include <sys/param.h>
#include <sys/system.h>
#include <sys/malloc.h>
#include <sys/mii.h>
#include <sys/mdio.h>
#include <sys/phy.h>
#include <fs/termio.h>

/* 全局PHY链表 */
static LIST_HEAD(phy_driver_list);

/* 读取PHY的厂商ID以及设备ID */
int get_phy_id(struct mii_bus *bus, uint8_t phyaddr, uint32_t *phy_id)
{
	uint32_t id;
	uint16_t regval;
	
	if(bus->mii_read(bus, phyaddr, MII_PHYSID1, &regval))
		return EIO;	
	if(0xffff == regval)
		return ENODEV;
	id = regval << 16;
	if(bus->mii_read(bus, phyaddr, MII_PHYSID2, &regval))
		return EIO;	
	if(0xffff == regval)
		return ENODEV;

	id |= regval;
	if(0x1fffffff == id)
		return ENODEV;

	*phy_id = id;

	return 0;
}

/* 创建一个PHY设备 */
struct phy_device *create_phy_device(struct mii_bus *bus, uint8_t phyaddr, uint32_t phy_id)
{
	struct phy_device *dev;

	if(!(dev = (struct phy_device *)kmem_zmalloc(sizeof(struct phy_device))))
		return NULL;
	
	dev->bus = bus;	
	dev->phy_addr = (uint32_t)phyaddr;
	dev->phy_id = phy_id;

	return dev;
}

/* 匹配PHY驱动 */
int phy_attach_driver(struct phy_device *dev)
{
	int m_maybe = 0, m;
	struct phy_driver *driver, *maybe = NULL;
	
	list_for_each_entry(driver, &phy_driver_list, list) {
		/* 查找最优匹配 */
		if((m = (*driver->phy_match)(dev, driver)) > m_maybe) {
			m_maybe = m;
			maybe = driver;
		}
	}

	dev->driver = maybe;
	if(!dev->driver)
		panic("phy driver null");

	return 0;
}

/* 链接PHY与网口驱动设备之间的联系，并调用PHY的初始化配置 */
struct phy_device *phy_connect(void *netdev, struct mii_bus *bus, uint8_t phyaddr)
{
	int ret;
	struct phy_device *phydev;
	
	if(!(phydev = bus->phy_map[phyaddr & PHY_MASK])) {
		return ERR_PTR(ENODEV);
	}
	/* 初始化PHY配置 */
	if((ret = (*phydev->driver->config_init)(phydev)) != 0)
		return ERR_PTR(ret);
	
	if(phydev->attachdev)
		return ERR_PTR(EBUSY);
	
	phydev->attachdev = netdev;
	
	return phydev;
}

/* 解除PHY与网口驱动设备之间的联系 */
void phy_disconnect(void *netdev, struct mii_bus *bus, uint8_t phyaddr)
{
	struct phy_device *phydev;
	
	if((phydev = bus->phy_map[phyaddr & PHY_MASK]) != NULL) {
		phydev->attachdev = NULL;
	}
}

/* 释放PHY空间 */
void free_phy_device(struct phy_device *dev)
{
	kmem_free(dev);
}

/* 通用PHY驱动定义实现 */
static int genphy_match(struct phy_device *dev, struct phy_driver *driver)
{
	/* 通用PHY永远返回匹配最低优先级的匹配 */
	return 1;
}

static int genphy_config_init(struct phy_device *dev)
{
	return 0;
}

static int genphy_read_status(struct phy_device *dev)
{
	return 0;
}

static struct phy_driver genphy_driver = 
{
	.features = 0xffffffff,
	.phy_match = genphy_match,
	.config_init = genphy_config_init,
	.read_status = genphy_read_status,
};

int	phy_driver_register(struct phy_driver *driver)
{
	list_add(&driver->list, &phy_driver_list);
	return 0;
}

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) phy_driver_init(void)
{
	phy_driver_register(&genphy_driver);
}
