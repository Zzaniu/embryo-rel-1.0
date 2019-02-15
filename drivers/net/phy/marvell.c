
/**
 * Copyright(c) 2018-2-24 Shangwen Wu	
 *
 * Marvell PHY驱动相关函数
 * 
 */
#include <common.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/list.h>
#include <sys/param.h>
#include <sys/system.h>
#include <sys/malloc.h>
#include <sys/mii.h>
#include <sys/mdio.h>
#include <sys/phy.h>
#include <sys/marvell_phy.h>
#include <fs/termio.h>

/* marvel PHY 厂家相关寄存器定义 */
#define MII_M1111_PHY_EXT_CR		0x14
#define MII_M1111_RX_DELAY			0x80
#define MII_M1111_TX_DELAY			0X02

/* 通用PHY驱动定义实现 */
static int marvell_match(struct phy_device *dev, struct phy_driver *driver)
{
	if((dev->phy_id & driver->phy_id_mask) == driver->phy_id)
		return 2;		//返回更高优先级

	return 0;
}

static int m88e1111_config_init(struct phy_device *dev)
{
	int err;
	uint16_t regval;

	log(LOG_DEBUG, "Detect Marvel 88E1111 PHY chip.\n");
	
	/* bad code 这里暂时仅初始化RGMII模式配置 */
	/**
 	 * 注意：此处需要先进行延时设置，然后再软复位操作，否则会出现描述符指针正常移动
 	 * 		 但是无法发送数据报文的情况（因为时序导致RGMII无法通信）
 	 */
	if(1/* phy mode = ? */) {
		/* set RGMII tx/rx 1.9ns delay*/
		phy_read(dev, MII_M1111_PHY_EXT_CR, &regval);
		regval |= (MII_M1111_RX_DELAY | MII_M1111_TX_DELAY);
		if((err = phy_write(dev, MII_M1111_PHY_EXT_CR, regval)))
			return err;
	}

	/* reset phy */
	phy_read(dev, MII_BMCR, &regval);
	regval |= BMCR_RESET;
	if((err = phy_write(dev, MII_BMCR, regval)) != 0)
		return err;

	return 0;
}

static int m88e1111_read_status(struct phy_device *dev)
{
	return 0;
}

static struct phy_driver marvell_phy_drivers[] = {
	{
		.phy_id = MARVELL_PHY_ID_88E1111,
		.phy_id_mask = MARVELL_PHY_ID_MASK,
		.name = "Marvel 88E1111",
		.features = 0xffffffff,
		.phy_match = marvell_match,
		.config_init = m88e1111_config_init,
		.read_status = m88e1111_read_status,
	},
};

/**
 * 描述：命令初始化函数
 */
static void __attribute__((constructor)) phy_driver_init(void)
{
	int i;
	
	for(i = 0; i < NR(marvell_phy_drivers); ++i)
		phy_driver_register(marvell_phy_drivers + i);
}
