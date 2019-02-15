
/**
 * Copyright(c) 2017-1-19 Shangwen Wu
 * stmmac(gmac)mdio操作
 * 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/system.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/mdio.h>
#include <sys/phy.h>
#include <fs/termio.h>

#include "stmmac.h"

/* 下面的定义对于1000和100/10MAC定义一致 */
#define GMAC_MII_ADDR			0x00000010	/* MII地址寄存器 */
#define GMAC_MII_DATA			0x00000014	/* MII数据寄存器 */

/* MII寄存器相关位域定义 */
#define GMAC_MII_PHY_MASK		0x0000f800	/* PHY地址掩码 */
#define GMAC_MII_PHY_SHIFT		11			/* PHY地址偏移 */
#define GMAC_MII_REG_MASK		0x000007c0	/* PHY寄存器掩码 */
#define GMAC_MII_REG_SHIFT		6			/* PHY寄存器偏移 */
#define GMAC_MII_CSRCLK_MASK	0x0000001c	/* MII通信时钟掩码 */
#define GMAC_MII_CSRCLK_250M_300MHz	0x00000014
#define GMAC_MII_CSRCLK_150M_250MHz	0x00000010
#define GMAC_MII_CSRCLK_35M_60MHz	0x0000000c
#define GMAC_MII_CSRCLK_20M_35MHz	0x00000008
#define GMAC_MII_CSRCLK_100M_150MHz	0x00000004
#define GMAC_MII_CSRCLK_60M_100MHz	0x00000000
#define GMAC_MII_READ			0x00000000	/* MII读命令 */
#define GMAC_MII_WRITE			0x00000002	/* MII读命令 */
#define GMAC_MII_BUSY			0x00000001	/* 传输状态位 */


extern uint32_t csrclk;	//defined in stmmac_main.c 

static int stmmac_mdio_read(struct mii_bus *bus, uint8_t phyaddr, uint8_t reg, uint16_t *val)
{
	struct stmmac_device *stmdev = (struct stmmac_device *)bus->mii_priv;
	uint16_t data = ((phyaddr << GMAC_MII_PHY_SHIFT) & GMAC_MII_PHY_MASK) |
		((reg << GMAC_MII_REG_SHIFT) & GMAC_MII_REG_MASK) | csrclk | GMAC_MII_READ;
	
	data |= GMAC_MII_BUSY;

	do {} while (stmmac_read_reg(stmdev, GMAC_MII_ADDR) & GMAC_MII_BUSY);
	stmmac_write_reg(stmdev, GMAC_MII_ADDR, (uint32_t)data);
	do {} while (stmmac_read_reg(stmdev, GMAC_MII_ADDR) & GMAC_MII_BUSY);

	*val = (uint16_t)stmmac_read_reg(stmdev, GMAC_MII_DATA);

	return 0;
}

static int stmmac_mdio_write(struct mii_bus *bus, uint8_t phyaddr, uint8_t reg, uint16_t val)
{
	struct stmmac_device *stmdev = (struct stmmac_device *)bus->mii_priv;
	uint16_t data = ((phyaddr << GMAC_MII_PHY_SHIFT) & GMAC_MII_PHY_MASK) |
		((reg << GMAC_MII_REG_SHIFT) & GMAC_MII_REG_MASK) | csrclk | GMAC_MII_WRITE;
	
	data |= GMAC_MII_BUSY;

	do {} while (stmmac_read_reg(stmdev, GMAC_MII_ADDR) & GMAC_MII_BUSY);
	stmmac_write_reg(stmdev, GMAC_MII_DATA, (uint32_t)val);
	stmmac_write_reg(stmdev, GMAC_MII_ADDR, (uint32_t)data);
	do {} while (stmmac_read_reg(stmdev, GMAC_MII_ADDR) & GMAC_MII_BUSY);

	return 0;
}

int stmmac_mdio_register(struct stmmac_device *stmdev)
{
	uint8_t i;
	struct mii_bus *bus;
	
	/* 注意这里初始化mdio访问频率 */
	csrclk = GMAC_MII_CSRCLK_35M_60MHz;

	if(!(bus = (struct mii_bus *)kmem_zmalloc(sizeof(struct mii_bus)))) {
		log(LOG_ERR, "mii_bus: out of memory\n");
		return ENOMEM;
	}

	strcpy(bus->mii_name, stmdev->dev.dv_cfdata->cf_driver->cd_name);
	bus->mii_id = stmdev->dev.dv_unit;
	bus->mii_priv = stmdev;
	bus->mii_read = stmmac_mdio_read;
	bus->mii_write = stmmac_mdio_write;
	mdio_register(bus);

	if(-1 == stmdev->phyaddr)
		for(i = 0; i < PHY_MAX; ++i) {
			if(bus->phy_map[i])
				stmdev->phyaddr = i;
		}

	stmdev->bus = bus;
	
	return 0;
}

void stmmac_mdio_unregister(struct stmmac_device *stmdev)
{
	struct mii_bus *bus = stmdev->bus;

	stmdev->bus = NULL;
	mdio_unregister(bus);
	kmem_free(bus);
}
