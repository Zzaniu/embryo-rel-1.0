
/**
 * Copyright(c) 2018-4-9 Shangwen Wu	
 *
 * stmmac网卡驱动头文件 
 * 
 */

#ifndef __STMMAC_H__
#define __STMMAC_H__

struct mii_bus;
struct ether_dev;
struct phy_device;
struct dma_desc;
struct stmmac_mac_ops;
struct stmmac_dma_ops;
struct stmmac_desc_ops;

/* 最大发送阀值 */
#define TC_DEFAULT		64

/* 发送FCS计算责任人 */
#define HW_CKSUM		1
#define SW_CKSUM		0

/* GMAC发送队列长度 */
#define TX_QUEUE_SIZE	8
#define RX_QUEUE_SIZE	8

/* DMA描述符环大小 */
#define TX_DMA_SIZE		8
#define RX_DMA_SIZE		8

struct mac_device_info {
	struct stmmac_mac_ops *mac;
	struct stmmac_dma_ops *dma;
	struct stmmac_desc_ops *desc;
	struct mac_link {
		uint32_t port;						//端口选择	
		uint32_t duplex;					//双工模式选择
		uint32_t speed;						//链路速度选择
	} link;
};

/* stmmac设备描述 */
struct stmmac_device {
	struct device dev;						//device结构一定要在最前面
	uint64_t iobase;						//网口寄存器基地址
	struct mii_bus *bus;					//mdio总线地址
	uint32_t phyaddr;						//PHY地址
	struct ether_dev *ethdev;				//以太网接口描述
	struct phy_device *phydev;				//PHY描述
	struct mac_device_info *hw;				//硬件描述信息
	void (*bsp_setup)(void *);				//平台相关初始化
	/* DMA描述符相关定义 */	
	struct dma_desc *dma_rx;				/* 描述符缓冲区 */
	struct dma_desc *dma_tx;
	dma_addr_t dma_rx_phy;					/* 描述符缓冲区对应的DMA地址 */
	dma_addr_t dma_tx_phy;
	/* 收发数据缓冲区 */
	uint8_t **buf_rx;						/* 保存tx/rx_ring_sz个数据缓冲区指针的数组 */
	uint8_t **buf_tx;
	//dma_addr_t *buf_rx_phy;				/* 每个数据缓冲区对应的DMA地址 */
	//dma_addr_t *buf_tx_phy;
	/* 描述符索引 */
	unsigned int cur_dma_rx;				/* 当前可用描述符索引位置 */
	unsigned int cur_dma_tx;
	//unsigned int dirty_dma_rx;				/* 当前属于忙碌状态（被DMA占用）的描述符 */
	unsigned int dirty_dma_tx;
};

/* stmmac驱动调试开关 */
#define STMMAC_DEBUG		0
#if STMMAC_DEBUG
#define STMMAC_DBG(fmt, args...)		printf(fmt, ##args)
#else
#define STMMAC_DBG(fmt, args...)		do{}while(0)	
#endif

extern int stmmac_init(struct stmmac_device *stmdev, uint32_t iobase);//defined in stmmac_main.c
extern void stmmac_exit(struct stmmac_device *stmdev);//defined in stmmac_main.c
extern int stmmac_mdio_register(struct stmmac_device *stmdev);//defined in stmmac_mdio.c
extern void stmmac_mdio_unregister(struct stmmac_device *stmdev);//defined in stmmac_mdio.c
extern uint32_t stmmac_read_reg(struct stmmac_device *stmdev, uint32_t reg);//defined in stmmac_hw.c
extern void stmmac_write_reg(struct stmmac_device *stmdev, uint32_t reg, uint32_t val);//defined in stmmac_hw.c

#endif //__STMMAC_H__
