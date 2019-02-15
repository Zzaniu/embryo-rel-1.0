
/**
 * Copyright(c) 2018-8-23 Shangwen Wu	
 *
 * stmmac 千兆网卡mac控制器部分操作实现 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <string.h>
#include <sys/err.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/device.h>

#include "dma_desc.h"
#include "common.h"
#include "stmmac.h"
#include "dwmac1000.h"


/**
 * 描述：初始化MAC控制器
 */
static void dwmac1000_core_init(struct stmmac_device *stmdev)
{
	uint32_t regval;

	/* 设置MAC configuration寄存器 */
	regval = stmmac_read_reg(stmdev, GMAC_CONTROL);
	//bad code 临时强制为1000M
	regval |= GMAC_CORE_INIT | GMAC_CONTROL_DM;		//1000M
	//regval |= GMAC_CORE_INIT | GMAC_CONTROL_PS | GMAC_CONTROL_DM | GMAC_CONTROL_FES;   //100M
	stmmac_write_reg(stmdev, GMAC_CONTROL, regval);
	
	/* bad code! receive all packet，不关闭下面地址过滤器将收不到单播报文 */
	regval = stmmac_read_reg(stmdev, GMAC_FRAME_FILTER);
	regval |= GMAC_FRAME_FILTER_RA;	
	stmmac_write_reg(stmdev, GMAC_FRAME_FILTER, regval);

	/* 关闭MMC计数 */
	stmmac_write_reg(stmdev, GMAC_MMC_CTRL, 0x8);
	/* 关闭GMAC中断 */
	stmmac_write_reg(stmdev, GMAC_INT_MASK, 0x20f);
	/* 关闭MMC中断 */
	stmmac_write_reg(stmdev, GMAC_MMC_HI_INTR_MASK, 0xffffffff);
	stmmac_write_reg(stmdev, GMAC_MMC_LO_INTR_MASK, 0xffffffff);
}

static void dwmac1000_flow_ctrl(struct stmmac_device *stmdev)
{

}

/**
 * 描述：设置MAC地址
 * 参数：addr：要设置的地址值，reg_n：mac组号
 */
static void dwmac1000_set_mac_addr(struct stmmac_device *stmdev, const uint8_t *addr, int reg_n)
{
	dwmac_set_mac_addr(stmdev, addr, GMAC_ADDR_HIGH(reg_n), GMAC_ADDR_LOW(reg_n));
}

static void dwmac_enable_tx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, GMAC_CONTROL);
	regval |= GMAC_CONTROL_TE;
	stmmac_write_reg(stmdev, GMAC_CONTROL, regval);
}

static void dwmac_enable_rx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, GMAC_CONTROL);
	regval |= GMAC_CONTROL_RE;
	stmmac_write_reg(stmdev, GMAC_CONTROL, regval);
}

static void dwmac_disable_tx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, GMAC_CONTROL);
	regval &= ~GMAC_CONTROL_TE;
	stmmac_write_reg(stmdev, GMAC_CONTROL, regval);
}

static void dwmac_disable_rx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, GMAC_CONTROL);
	regval &= ~GMAC_CONTROL_RE;
	stmmac_write_reg(stmdev, GMAC_CONTROL, regval);
}

/**
 * 描述：获取硬件寄存器的MAC地址
 * 参数：addr：返回地址值，reg_n：mac组号
 */
static void dwmac1000_get_mac_addr(struct stmmac_device *stmdev, uint8_t *addr, int reg_n)
{
	dwmac_get_mac_addr(stmdev, addr, GMAC_ADDR_HIGH(reg_n), GMAC_ADDR_LOW(reg_n));
}

/**
 * 描述：GMAC中断处理
 */
void dwmac1000_host_interrupt(struct stmmac_device *stmdev)
{
	uint32_t status;

	status = stmmac_read_reg(stmdev, GMAC_INT_STATUS);
	//STMMAC_DBG("%s, GMAC irq status 0x%x\n", __func__, status);

	if(status & mmc_rx_irq)
		STMMAC_DBG("%s: GMAC MMC rx interrupt: 0x%x\n", __func__, stmmac_read_reg(stmdev, GMAC_MMC_RX_INTR));
	if(status & mmc_tx_irq)
		STMMAC_DBG("%s: GMAC MMC tx interrupt: 0x%x\n", __func__, stmmac_read_reg(stmdev, GMAC_MMC_TX_INTR));
	if(status & mmc_rx_csum_offload_irq)
		STMMAC_DBG("%s: GMAC MMC rx csum offload interrupt: 0x%x\n", __func__, stmmac_read_reg(stmdev, GMAC_MMC_RX_CSUM_OFFLOAD));
	if(status & pmt_irq)
		STMMAC_DBG("%s: GMAC PMT interrupt: 0x%x\n", __func__, stmmac_read_reg(stmdev, GMAC_PMT));
	if(status & rgmii_irq)
		STMMAC_DBG("%s: GMAC RGMII interrupt: 0x%x\n", __func__, stmmac_read_reg(stmdev, GMAC_GMII_STATUS));
}

/* stmmac网卡mac控制器操作 */
struct stmmac_mac_ops dwmac1000_mac_ops = {
	.core_init = dwmac1000_core_init,
	.flow_ctrl = dwmac1000_flow_ctrl,
	.set_mac_addr = dwmac1000_set_mac_addr,
	.get_mac_addr = dwmac1000_get_mac_addr,
	.enable_tx = dwmac_enable_tx,
	.enable_rx = dwmac_enable_rx,
	.disable_tx = dwmac_disable_tx,
	.disable_rx = dwmac_disable_rx,
	.host_interrupt = dwmac1000_host_interrupt,
};

/**
 * 描述：设置千兆网卡相关参数
 */ 
void dwmac1000_info_setup(struct stmmac_device *stmdev, struct mac_device_info *info)
{
	uint32_t uid = stmmac_read_reg(stmdev, GMAC_VERSION);

	printf("DWMAC1000 -user ID: 0x%x, Synopsys ID: 0x%x\n", 
				(uid & 0x0000ff00) >> 8, uid & 0x000000ff);

	info->mac = &dwmac1000_mac_ops;
	info->dma = &dwmac1000_dma_ops;
	info->link.port = GMAC_CONTROL_PS;
	info->link.speed= GMAC_CONTROL_FES;
	info->link.duplex = GMAC_CONTROL_DM;
}

