
/**
 * Copyright(c) 2018-8-23 Shangwen Wu	
 *
 * stmmac 网卡公共处理函数
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
#include "dwmac_dma.h"

void dwmac_dma_start_tx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, DMA_CONTROL);
	regval |= DMA_CONTROL_ST;
	stmmac_write_reg(stmdev, DMA_CONTROL, regval);
}

void dwmac_dma_start_rx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, DMA_CONTROL);
	regval |= DMA_CONTROL_SR;
	stmmac_write_reg(stmdev, DMA_CONTROL, regval);
}

void dwmac_dma_stop_tx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, DMA_CONTROL);
	regval &= ~DMA_CONTROL_ST;
	stmmac_write_reg(stmdev, DMA_CONTROL, regval);
}

void dwmac_dma_stop_rx(struct stmmac_device *stmdev)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, DMA_CONTROL);
	regval &= ~DMA_CONTROL_SR;
	stmmac_write_reg(stmdev, DMA_CONTROL, regval);
}

void dwmac_enable_dma_transmission(struct stmmac_device *stmdev)
{
	stmmac_write_reg(stmdev, DMA_XMT_POLL_DEMAND, 1);
}

int dwmac_dma_interrupt(struct stmmac_device *stmdev)
{
	int ret = no_extra;
	uint32_t status;
	
	status = stmmac_read_reg(stmdev, DMA_STATUS);
	//STMMAC_DBG("stmmac DMA irq status 0x%x\n", status);

	if(status & DMA_STATUS_AIS) {
		STMMAC_DBG("%s: DMA Abnormal IRQ: ", __func__);
		if(status & DMA_STATUS_UNF) {
			STMMAC_DBG("transmit underflow ");
			ret = tx_hard_error;
		}
		if(status & DMA_STATUS_TJT) {
			STMMAC_DBG("transmit jabber ");
		}
		if(status & DMA_STATUS_OVF) {
			STMMAC_DBG("receive overflow ");
		}
		if(status & DMA_STATUS_RU) {
			STMMAC_DBG("receive buffer unavailable ");
		}
		if(status & DMA_STATUS_RPS) {
			STMMAC_DBG("receive process stopped ");
		}
		if(status & DMA_STATUS_RWT) {
			STMMAC_DBG("receive watchdog ");
		}
		if(status & DMA_STATUS_ETI) {
			STMMAC_DBG("transmit early interrupt ");
		}
		if(status & DMA_STATUS_TPS) {
			STMMAC_DBG("transmit process stopped ");
			ret = tx_hard_error;
		}
		if(status & DMA_STATUS_FBI) {
			STMMAC_DBG("fatal bus error ");
			ret = tx_hard_error;
		}
		STMMAC_DBG("\n");
	}

	if(status & DMA_STATUS_NIS) {
		if((status & DMA_STATUS_RI) || (status & DMA_STATUS_TI))
			ret = handle_tx_rx;
#if 1
		/**
		 * 注意：这里出现偶尔丢失中断完成标志位的情况，因此在这里认为如果接收到
		 * 		 "early receive"中断，那么也认为一次接收完成，
		 *		bug!!!
		 */
		else if(status & DMA_STATUS_ERI) 
			ret = handle_tx_rx;
#endif
	}
	
	if(status & (DMA_STATUS_GPI | DMA_STATUS_GMI | DMA_STATUS_GLI))
		STMMAC_DBG("stmmac: unexpected irq status 0x%x\n", status);
	
	stmmac_write_reg(stmdev, DMA_STATUS, status & 0x1ffff);

	return ret;
}

void dwmac_set_mac_addr(struct stmmac_device *stmdev, const uint8_t *addr, 
				unsigned long hi, unsigned long lo)
{
	uint32_t regval;

	regval = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	stmmac_write_reg(stmdev, lo, regval);
	regval = (addr[5] << 8) | addr[4];
	stmmac_write_reg(stmdev, hi, regval);
}

void dwmac_get_mac_addr(struct stmmac_device *stmdev, uint8_t *addr, 
				unsigned long hi, unsigned long lo)
{
	uint32_t regval;

	regval = stmmac_read_reg(stmdev, lo);
	addr[3] = (regval >> 24) & 0xff;
	addr[2] = (regval >> 16) & 0xff;
	addr[1] = (regval >> 8) & 0xff;
	addr[0] = regval & 0xff;
	regval = stmmac_read_reg(stmdev, hi);
	addr[5] = (regval >> 8) & 0xff;
	addr[4] = regval & 0xff;
}

