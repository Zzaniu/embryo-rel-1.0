
/**
 * Copyright(c) 2018-8-23 Shangwen Wu	
 *
 * stmmac 千兆网卡mac控制器部分操作实现 
 * 
 */
#include <common.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/device.h>

#include "dma_desc.h"
#include "common.h"
#include "stmmac.h"

static int enh_read_tx_status(struct dma_desc *d)
{
	int ret = 0;

	if(d->des01.etx.error_summary) {
		STMMAC_DBG("%s: GMAC tx error\n", __func__);	
		ret = -1;
	}

	return ret;
}

static int enh_read_rx_status(struct dma_desc *d)
{
	int ret = 0;

	if(d->des01.erx.error_summary) {
		STMMAC_DBG("%s: GMAC rx error\n", __func__);	
		ret = -1;
	}

	return ret;
}

static void enh_init_tx_desc(struct dma_desc *d, unsigned int ring_sz)
{
	unsigned int i;

	for(i = 0; i < ring_sz; ++i) {
		d->des01.etx.own = 0;
		if(i == (ring_sz - 1))
			d->des01.etx.end_ring = 1;
		++d;
	}
}

static void enh_init_rx_desc(struct dma_desc *d, unsigned int ring_sz, int dis_intr)
{
	unsigned int i;

	for(i = 0; i < ring_sz; ++i) {
		d->des01.erx.own = 1;
		d->des01.erx.buffer1_size = BUF_SIZE_8KB - 1;
		d->des01.erx.buffer2_size = BUF_SIZE_8KB - 1;
		if(i == (ring_sz - 1))
			d->des01.erx.end_ring = 1;
		if(dis_intr)
			d->des01.erx.disable_ic= 1;
		++d;
	}
}

static void enh_set_tx_owner(struct dma_desc *d)
{
	d->des01.etx.own = 1;
}

static void enh_set_rx_owner(struct dma_desc *d)
{
	d->des01.erx.own = 1;
}

static int enh_get_tx_owner(struct dma_desc *d)
{
	return d->des01.etx.own;
}

static int enh_get_rx_owner(struct dma_desc *d)
{
	return d->des01.erx.own;
}

static uint32_t enh_get_tx_len(struct dma_desc *d)
{
	return d->des01.etx.buffer1_size;
}

static uint32_t enh_get_rx_frame_len(struct dma_desc *d)
{
	return d->des01.erx.frame_length;
}

static void enh_prepare_tx_desc(struct dma_desc *d, int is_first, unsigned int len, int need_cksum)
{
	d->des01.etx.first_segment = !!is_first;

	if(len > BUF_SIZE_4KB) {
		d->des01.etx.buffer1_size = BUF_SIZE_4KB;
		d->des01.etx.buffer2_size = len - BUF_SIZE_4KB;
	} else 
		d->des01.etx.buffer1_size = len;

	if(need_cksum) {
		/* 软件计算除IP头外的其他校验和（包括负载以及伪头部校验和） */
		//d->des01.etx.checksum_insertion = CIC_FULL;
		d->des01.etx.checksum_insertion = CIC_ONLY_IP;
	}
}

static void enh_set_tx_last_desc(struct dma_desc *d)
{
	d->des01.etx.last_segment = 1;
	d->des01.etx.interrupt = 1;
}

static int enh_is_tx_last_desc(struct dma_desc *d)
{
	return d->des01.etx.last_segment;
}

static int enh_is_rx_last_desc(struct dma_desc *d)
{
	return d->des01.erx.last_descriptor;
}

static int enh_is_rx_first_desc(struct dma_desc *d)
{
	return d->des01.erx.first_descriptor;
}

static void enh_release_tx_desc(struct dma_desc *d)
{
	int end = d->des01.etx.end_ring;

	bzero(d, sizeof(struct dma_desc));
	d->des01.etx.end_ring= end;
}

/* stmmac网卡dma描述符操作 */
struct stmmac_desc_ops enh_desc_ops = {
	.read_tx_status = enh_read_tx_status,
	.read_rx_status = enh_read_rx_status,
	.init_tx_desc = enh_init_tx_desc, 
	.init_rx_desc = enh_init_rx_desc,
	.set_tx_owner = enh_set_tx_owner, 
	.set_rx_owner = enh_set_rx_owner,
	.get_tx_owner = enh_get_tx_owner,
	.get_rx_owner = enh_get_rx_owner,
	.get_tx_len = enh_get_tx_len,
	.get_rx_frame_len = enh_get_rx_frame_len,
	.prepare_tx_desc = enh_prepare_tx_desc,
	.set_tx_last_desc = enh_set_tx_last_desc,
	.is_tx_last_desc = enh_is_tx_last_desc,
	.is_rx_last_desc = enh_is_rx_last_desc,
	.is_rx_first_desc = enh_is_rx_first_desc,
	.release_tx_desc = enh_release_tx_desc,
};

