
/**
 * Copyright(c) 2018-8-23 linux-2.6.32 stmmac driver & Shangwen Wu	
 *
 * stmmac网卡DMA描述符位域定义头文件 
 * 
 */

#ifndef __STMMAC_DMA_DESC_H__
#define __STMMAC_DMA_DESC_H__

struct dma_desc {
	union {
		/* Receive descriptor */
		struct {
			/* RDES0 */
			u32 reserved1:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 mii_error:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 collision:1;
			u32 frame_too_long:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 multicast_frame:1;
			u32 run_frame:1;
			u32 length_error:1;
			u32 partial_frame_error:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 filtering_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved2:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 reserved3:5;
			u32 disable_ic:1;
		} rx;
		struct {
			/* RDES0 */
			u32 payload_csum_error:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 error_gmii:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 late_collision:1;
			u32 ipc_csum_error:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 vlan_tag:1;
			u32 overflow_error:1;
			u32 length_error:1;
			u32 sa_filter_fail:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 da_filter_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:13;
			u32 reserved1:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 buffer2_size:13;
			u32 reserved2:2;
			u32 disable_ic:1;
		} erx;		/* -- enhanced -- */

		/* Transmit descriptor */
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 heartbeat_fail:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 reserved1:3;
			u32 error_summary:1;
			u32 reserved2:15;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved3:1;
			u32 disable_padding:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 crc_disable:1;
			u32 reserved4:2;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
		} tx;
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 vlan_frame:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 payload_error:1;
			u32 frame_flushed:1;
			u32 jabber_timeout:1;
			u32 error_summary:1;
			u32 ip_header_error:1;
			u32 time_stamp_status:1;
			u32 reserved1:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 checksum_insertion:2;
			u32 reserved2:1;
			u32 time_stamp_enable:1;
			u32 disable_padding:1;
			u32 crc_disable:1;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:13;
			u32 reserved3:3;
			u32 buffer2_size:13;
			u32 reserved4:3;
		} etx;		/* -- enhanced -- */
	} des01;
	unsigned int des2;
	unsigned int des3;
};

enum tx_desc_chksum_insertion {
	CIC_DISABLE = 0,			/* 硬件不填充任何校验 */
	CIC_ONLY_IP = 1,			/* 硬件仅填充IP头校验和 */
	CIC_NO_PSEUDOHEADER = 2,	/* 硬件填充IP头和负载校验和，但不计算伪头校验和 */
	CIC_FULL = 3,				/* 硬件填充IP头和负载校验和，计算伪头校验和 */
};

#endif //__STMMAC_DMA_DESC_H__
