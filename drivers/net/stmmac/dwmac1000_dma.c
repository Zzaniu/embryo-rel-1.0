
/**
 * Copyright(c) 2018-8-23 Shangwen Wu	
 *
 * stmmac 千兆网卡dma控制器部分操作实现 
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
#include "dwmac_dma.h"

/**
 * 描述：DMA初始化
 * 参数：tx_dma_phy, rx_dma_phy：收发描述符的DMA地址;pbl：突发长度
 */
static int dwmac1000_dma_init(struct stmmac_device *stmdev, uint32_t pbl,
			dma_addr_t tx_dma_phy, dma_addr_t rx_dma_phy)
{
	uint32_t timeout = 100000;
	uint32_t regval;

	/* 软复位GMAC & DMA */
	regval = stmmac_read_reg(stmdev, DMA_BUS_MODE);
	regval |= DMA_BUS_MODE_SFT_RESET;
	stmmac_write_reg(stmdev, DMA_BUS_MODE, regval);
	/* 等待复位完成 */
	do {
		//do nothing
	} while((stmmac_read_reg(stmdev, DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET) && timeout--);
	if(!timeout)
		return ETIMEDOUT;

	/* 设置传输突发长度 */
	regval = DMA_BUS_MODE_8XPBL | (pbl << DMA_BUS_MODE_PBL_SHIFT) | (pbl << DMA_BUS_MODE_RPBL_SHIFT);
	stmmac_write_reg(stmdev, DMA_BUS_MODE, regval);
	
	/* 使能汇总DMA中断 */
	stmmac_write_reg(stmdev, DMA_INTR_ENA, DMA_INTR_DEFAULT_MASK);
	
	/* 写入描述符DMA基地值到控制器 */
	stmmac_write_reg(stmdev, DMA_TX_BASE_ADDR, tx_dma_phy);
	stmmac_write_reg(stmdev, DMA_RCV_BASE_ADDR, rx_dma_phy);

	return 0;
}

/**
 * 描述：设置DMA操作模式
 */
static void dwmac1000_dma_mode(struct stmmac_device *stmdev, int txmode, int rxmode)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, DMA_CONTROL);
	
	/* 发送模式 */	
	if(SF_DMA_MODE == txmode) {
		regval |= DMA_CONTROL_TSF;
		regval |= DMA_CONTROL_OSF;
	} else {
		regval &= ~DMA_CONTROL_TSF;
		regval &= DMA_CONTROL_TC_TX_MASK;
		if(txmode <= 32)
			regval |= DMA_CONTROL_TTC_32;
		else if(txmode <= 64)
			regval |= DMA_CONTROL_TTC_64;
		else if(txmode <= 128)
			regval |= DMA_CONTROL_TTC_128;
		else if(txmode <= 192)
			regval |= DMA_CONTROL_TTC_192;
		else
			regval |= DMA_CONTROL_TTC_256;
	}
	/* 接收模式 */
	if(SF_DMA_MODE == rxmode) {
		regval |= DMA_CONTROL_RSF;
	} else {
		regval &= ~DMA_CONTROL_RSF;
		regval &= DMA_CONTROL_TC_RX_MASK;
		if(rxmode <= 32)
			regval |= DMA_CONTROL_RTC_32;
		else if(rxmode <= 64)
			regval |= DMA_CONTROL_RTC_64;
		else if(rxmode <= 96)
			regval |= DMA_CONTROL_RTC_96;
		else
			regval |= DMA_CONTROL_RTC_128;
	}

	stmmac_write_reg(stmdev, DMA_CONTROL, regval);
}

/* stmmac网卡dma控制器操作 */
struct stmmac_dma_ops dwmac1000_dma_ops = {
	.dma_init = dwmac1000_dma_init, 
	.dma_mode = dwmac1000_dma_mode,
	.start_tx = dwmac_dma_start_tx,
	.start_rx = dwmac_dma_start_rx,
	.stop_tx = dwmac_dma_stop_tx,
	.stop_rx = dwmac_dma_stop_rx,
	.enable_dma_transmission = dwmac_enable_dma_transmission,
	.dma_interrupt = dwmac_dma_interrupt,
};

