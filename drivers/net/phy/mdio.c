
/**
 * Copyright(c) 2018-4-9 Shangwen Wu	
 *
 * mdio PHY访问相关函数
 * 
 */
#include <common.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/mdio.h>
#include <sys/phy.h>
#include <fs/termio.h>

/* 全局mii_bus链表 */
static LIST_HEAD(miibus_list);

int mdio_read(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t *val)
{
	struct mii_bus *bus;

	if(!(bus = mdio_match(name))) {
		log(LOG_ERR, "mii-bus of %s not found!\n", name);
		errno = ENODEV;
		return -1;
	}

	if(errno = bus->mii_read(bus, phyaddr, reg, val))
		return -1;
	
	return 0;
}

int mdio_write(const char *name, uint8_t phyaddr, uint8_t reg, uint16_t val)
{
	struct mii_bus *bus;

	if(!(bus = mdio_match(name))) {
		log(LOG_ERR, "mii-bus of %s not found!\n", name);
		errno = ENODEV;
		return -1;
	}

	if(errno = bus->mii_write(bus, phyaddr, reg, val))
		return -1;

	return 0;
}

/* 根据网络接口名查找匹配的mii_bus */
struct mii_bus *mdio_match(const char *devname)
{
	struct mii_bus *bus;
	size_t dlen = strlen(devname), len;
	unsigned long id;

	list_for_each_entry(bus, &miibus_list, mii_list) {
		len = strlen(bus->mii_name);
		if(dlen <= len)
			continue;
		if(strncmp(bus->mii_name, devname, len))
			continue;
		if(atob(&id, devname + len, 10))
			continue;
		if(id == bus->mii_id)
			return bus;
	}

	return NULL;
}

/* 根据指定的PHY地址创建对应miibus上的PHY设备 */
int mdio_scan(struct mii_bus *bus, uint8_t phyaddr)
{
	int err;
	uint32_t phyid;
	struct phy_device *dev;
		
	if((err = get_phy_id(bus, phyaddr, &phyid)))
		return err;

	if(!(dev = create_phy_device(bus, phyaddr, phyid)))
		return ENOMEM;

	phy_attach_driver(dev);
	
	bus->phy_map[phyaddr & PHY_MASK] = dev;
	
	return 0;
}

/* 注册mdio函数 */
int mdio_register(struct mii_bus *bus)
{
	int err;
	uint8_t i;

	for(i = 0; i < PHY_MAX; ++i)
		if(err = mdio_scan(bus, i)) {	
			if(err != ENODEV)
				log(LOG_ERR, "mdio_register: scan phyaddr 0x%x err %d\n", (uint32_t)i, errno);	
		}

	list_add(&bus->mii_list, &miibus_list);
	return 0;
}

int mdio_unregister(struct mii_bus *bus)
{
	uint8_t i;

	for(i = 0; i < PHY_MAX; ++i)
		if(bus->phy_map[i]) { 
			free_phy_device(bus->phy_map[i]);
			bus->phy_map[i] = NULL;
		}

	list_del(&bus->mii_list);
	return 0;
}
