
/**
 * Copyright(c) 2018-8-23 Shangwen Wu	
 *
 * stmmac网卡公共定义头文件 
 * 
 */

#ifndef __STMMAC_COMMON_H__
#define __STMMAC_COMMON_H__

struct stmmac_device;
struct mac_device_info;
struct dma_desc;

/* skb缓冲区大小 */
#define BUF_SIZE_16KB	16384
#define BUF_SIZE_8KB	8192
#define BUF_SIZE_4KB 	4096	
#define BUF_SIZE_2KB 	2048

#define SF_DMA_MODE		1	/* 存储-转发 DMA操作模式 */

/* DMA中断状态类别 */
enum dma_irq_status {
	no_extra = 0,			//无需额外处理
	tx_hard_error = 1,		//需要处理tx错误
	handle_tx_rx = 2,		//需要进行收发处理
};

/* stmmac网卡mac控制器操作 */
struct stmmac_mac_ops {
	void (*core_init)(struct stmmac_device *);
	void (*flow_ctrl)(struct stmmac_device *);
	void (*set_mac_addr)(struct stmmac_device *, const uint8_t *, int);
	void (*get_mac_addr)(struct stmmac_device *, uint8_t *, int);
	void (*enable_tx)(struct stmmac_device *);
	void (*enable_rx)(struct stmmac_device *);
	void (*disable_tx)(struct stmmac_device *);
	void (*disable_rx)(struct stmmac_device *);
	void (*host_interrupt)(struct stmmac_device *);
};

/* stmmac网卡dma控制器操作 */
struct stmmac_dma_ops {
	int (*dma_init)(struct stmmac_device *, uint32_t, dma_addr_t, dma_addr_t);
	void (*dma_mode)(struct stmmac_device *, int, int);
	void (*start_tx)(struct stmmac_device *);
	void (*start_rx)(struct stmmac_device *);
	void (*stop_tx)(struct stmmac_device *);
	void (*stop_rx)(struct stmmac_device *);
	void (*enable_dma_transmission)(struct stmmac_device *);
	int (*dma_interrupt)(struct stmmac_device *);
};

/* stmmac网卡dma描述符操作 */
struct stmmac_desc_ops {
	int (*read_tx_status)(struct dma_desc *);
	int (*read_rx_status)(struct dma_desc *);
	void (*init_tx_desc)(struct dma_desc *, unsigned int);
	void (*init_rx_desc)(struct dma_desc *, unsigned int, int);
	void (*set_tx_owner)(struct dma_desc *);
	void (*set_rx_owner)(struct dma_desc *);
	int (*get_tx_owner)(struct dma_desc *);
	int (*get_rx_owner)(struct dma_desc *);
	uint32_t (*get_tx_len)(struct dma_desc *);
	uint32_t (*get_rx_frame_len)(struct dma_desc *);
	void (*prepare_tx_desc)(struct dma_desc *, int , unsigned int, int);
	void (*set_tx_last_desc)(struct dma_desc *);
	int (*is_tx_last_desc)(struct dma_desc *);
	int (*is_rx_last_desc)(struct dma_desc *);
	int (*is_rx_first_desc)(struct dma_desc *);
	void (*release_tx_desc)(struct dma_desc *);
};

extern struct stmmac_desc_ops enh_desc_ops;//definded in enh_desc.c

extern void dwmac1000_info_setup(struct stmmac_device *, struct mac_device_info *);//defined in dwmac1000_mac.c
extern void dwmac_set_mac_addr(struct stmmac_device *, const uint8_t *, unsigned long, unsigned long);//define in dwmac_lib.c
extern void dwmac_get_mac_addr(struct stmmac_device *, uint8_t *addr, unsigned long, unsigned long);//define in dwmac_lib.c

#endif //__COMMON_H__
