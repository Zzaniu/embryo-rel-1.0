
/**
 * Copyright(c) 2018-8-24 @linux-2.6.32 stmmac driver & Shangwen Wu	
 *
 * stmmac DMA寄存器定义头文件 
 * 
 */

#ifndef __STMMAC_DWMAC_DMA_H__
#define __STMMAC_DWMAC_DMA_H__

struct stmmac_device;

/* DMA CRS Control and Status Register Mapping */
#define DMA_BUS_MODE		0x00001000	/* Bus Mode */
#define DMA_XMT_POLL_DEMAND	0x00001004	/* Transmit Poll Demand */
#define DMA_RCV_POLL_DEMAND	0x00001008	/* Received Poll Demand */
#define DMA_RCV_BASE_ADDR	0x0000100c	/* Receive List Base */
#define DMA_TX_BASE_ADDR	0x00001010	/* Transmit List Base */
#define DMA_STATUS			0x00001014	/* Status Register */
#define DMA_CONTROL			0x00001018	/* Ctrl (Operational Mode) */
#define DMA_INTR_ENA		0x0000101c	/* Interrupt Enable */
#define DMA_MISSED_FRAME_CTR	0x00001020	/* Missed Frame Counter */
#define DMA_CUR_TX_DESC		0x00001048	/* Current Host Tx Descriptor */
#define DMA_CUR_RX_DESC		0x0000104c	/* Current Host Rx Descriptor */
#define DMA_CUR_TX_BUF_ADDR	0x00001050	/* Current Host Tx Buffer */
#define DMA_CUR_RX_BUF_ADDR	0x00001054	/* Current Host Rx Buffer */

/* DMA Control register defines */
#define DMA_CONTROL_ST		0x00002000	/* Start/Stop Transmission */
#define DMA_CONTROL_SR		0x00000002	/* Start/Stop Receive */

/* DMA Normal interrupt */
#define DMA_INTR_ENA_NIE 	0x00010000	/* Normal Summary */
#define DMA_INTR_ENA_TIE 	0x00000001	/* Transmit Interrupt */
#define DMA_INTR_ENA_TUE 	0x00000004	/* Transmit Buffer Unavailable */
#define DMA_INTR_ENA_RIE 	0x00000040	/* Receive Interrupt */
#define DMA_INTR_ENA_ERE 	0x00004000	/* Early Receive */

/**
 * 注意：这里打开“早期接收中断”，bug!!!
 */
#if 1
#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE | DMA_INTR_ENA_ERE)
#else
#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE)
#endif

/* DMA Abnormal interrupt */
#define DMA_INTR_ENA_AIE 	0x00008000	/* Abnormal Summary */
#define DMA_INTR_ENA_FBE 	0x00002000	/* Fatal Bus Error */
#define DMA_INTR_ENA_ETE 	0x00000400	/* Early Transmit */
#define DMA_INTR_ENA_RWE 	0x00000200	/* Receive Watchdog */
#define DMA_INTR_ENA_RSE 	0x00000100	/* Receive Stopped */
#define DMA_INTR_ENA_RUE 	0x00000080	/* Receive Buffer Unavailable */
#define DMA_INTR_ENA_UNE 	0x00000020	/* Tx Underflow */
#define DMA_INTR_ENA_OVE 	0x00000010	/* Receive Overflow */
#define DMA_INTR_ENA_TJE 	0x00000008	/* Transmit Jabber */
#define DMA_INTR_ENA_TSE 	0x00000002	/* Transmit Stopped */

#define DMA_INTR_ABNORMAL	(DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE | \
				DMA_INTR_ENA_UNE)

/* DMA default interrupt mask */
#define DMA_INTR_DEFAULT_MASK	(DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)

/* DMA Status register defines */
#define DMA_STATUS_GPI		0x10000000	/* PMT interrupt */
#define DMA_STATUS_GMI		0x08000000	/* MMC interrupt */
#define DMA_STATUS_GLI		0x04000000	/* GMAC Line interface int */
#define DMA_STATUS_GMI		0x08000000
#define DMA_STATUS_GLI		0x04000000
#define DMA_STATUS_EB_MASK	0x00380000	/* Error Bits Mask */
#define DMA_STATUS_EB_TX_ABORT	0x00080000	/* Error Bits - TX Abort */
#define DMA_STATUS_EB_RX_ABORT	0x00100000	/* Error Bits - RX Abort */
#define DMA_STATUS_TS_MASK	0x00700000	/* Transmit Process State */
#define DMA_STATUS_TS_SHIFT	20
#define DMA_STATUS_RS_MASK	0x000e0000	/* Receive Process State */
#define DMA_STATUS_RS_SHIFT	17
#define DMA_STATUS_NIS		0x00010000	/* Normal Interrupt Summary */
#define DMA_STATUS_AIS		0x00008000	/* Abnormal Interrupt Summary */
#define DMA_STATUS_ERI		0x00004000	/* Early Receive Interrupt */
#define DMA_STATUS_FBI		0x00002000	/* Fatal Bus Error Interrupt */
#define DMA_STATUS_ETI		0x00000400	/* Early Transmit Interrupt */
#define DMA_STATUS_RWT		0x00000200	/* Receive Watchdog Timeout */
#define DMA_STATUS_RPS		0x00000100	/* Receive Process Stopped */
#define DMA_STATUS_RU		0x00000080	/* Receive Buffer Unavailable */
#define DMA_STATUS_RI		0x00000040	/* Receive Interrupt */
#define DMA_STATUS_UNF		0x00000020	/* Transmit Underflow */
#define DMA_STATUS_OVF		0x00000010	/* Receive Overflow */
#define DMA_STATUS_TJT		0x00000008	/* Transmit Jabber Timeout */
#define DMA_STATUS_TU		0x00000004	/* Transmit Buffer Unavailable */
#define DMA_STATUS_TPS		0x00000002	/* Transmit Process Stopped */
#define DMA_STATUS_TI		0x00000001	/* Transmit Interrupt */
#define DMA_CONTROL_FTF		0x00100000 /* Flush transmit FIFO */

extern void dwmac_dma_start_tx(struct stmmac_device *);//defined in dwmac_lib.c
extern void dwmac_dma_stop_tx(struct stmmac_device *);
extern void dwmac_dma_start_rx(struct stmmac_device *);
extern void dwmac_dma_stop_rx(struct stmmac_device *);
extern void dwmac_enable_dma_transmission(struct stmmac_device *);
extern int dwmac_dma_interrupt(struct stmmac_device *);

#endif //__STMMAC_DWMAC_DMA_H__
